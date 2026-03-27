/**
 * users.js - 用户管理 (Admin Only)
 */

let currentPage = 1;
const pageSize = 10;

document.addEventListener('DOMContentLoaded', () => {
  if (!api.requireAuth()) return;
  
  const user = api.getUser();
  if (!user || user.role !== 'admin') {
    toast.error('您没有权限访问此页面');
    setTimeout(() => {
      window.location.href = '/pages/dashboard.html';
    }, 1500);
    return;
  }
  
  renderSidebar('users');
  loadUsers();

  document.getElementById('searchKeyword').addEventListener('keypress', (e) => {
    if (e.key === 'Enter') {
      currentPage = 1;
      loadUsers();
    }
  });
});

/** 加载用户列表 */
async function loadUsers(page = currentPage) {
  currentPage = page;
  const tbody = document.getElementById('userTableBody');
  tbody.innerHTML = '<tr><td colspan="5"><div class="loading"><div class="spinner"></div></div></td></tr>';

  try {
    const res = await api.get(`/users?page=${page}&size=${pageSize}`);
    if (res.code === 200 && res.data && res.data.list) {
      renderTable(res.data.list);
      renderPagination(res.data.total, page, pageSize, loadUsers);
    } else {
      tbody.innerHTML = `<tr><td colspan="5" class="empty-state">加载失败: ${res.msg}</td></tr>`;
    }
  } catch (err) {
    tbody.innerHTML = '<tr><td colspan="5" class="empty-state">网络错误</td></tr>';
  }
}

function renderTable(list) {
  const tbody = document.getElementById('userTableBody');
  if (!list.length) {
    tbody.innerHTML = '<tr><td colspan="5"><div class="empty-state">暂无用户数据</div></td></tr>';
    return;
  }

  let html = '';
  list.forEach(u => {
    const roleBadge = u.role_id === 1 ? 'badge-danger' : (u.role_id === 2 ? 'badge-info' : 'badge-success');
    const roleText = u.role_name || (u.role_id === 1 ? '管理员' : (u.role_id === 2 ? '医生' : '护士'));
    
    html += `
      <tr>
        <td>${u.id}</td>
        <td>
          <div style="font-weight: 500">${u.username}</div>
          <div style="font-size: 0.8rem; color: var(--text-muted)">${u.real_name || ''}</div>
        </td>
        <td><span class="badge ${roleBadge}">${roleText}</span></td>
        <td>${u.created_at || '-'}</td>
        <td>
          <button class="btn btn-secondary btn-sm" onclick="openEditUser(${u.id}, '${u.username}', '${u.real_name || ''}', '${u.phone || ''}', ${u.role_id})">编辑</button>
          ${u.id !== api.getUser().id ? `<button class="btn btn-danger btn-sm" onclick="deleteUser(${u.id})">删除</button>` : `<button class="btn btn-secondary btn-sm" disabled>当前</button>`}
        </td>
      </tr>
    `;
  });
  tbody.innerHTML = html;
}

// 分页组件渲染
function renderPagination(total, page, size, callback) {
  const totalPages = Math.ceil(total / size) || 1;
  const container = document.getElementById('pagination');
  
  let html = `
    <button ${page <= 1 ? 'disabled' : ''} onclick="changePage(${page - 1})">上一页</button>
    <span class="page-info">${page} / ${totalPages}</span>
    <button ${page >= totalPages ? 'disabled' : ''} onclick="changePage(${page + 1})">下一页</button>
  `;
  container.innerHTML = html;

  // 将 changePage 挂载到 window，为了能在原生的 onclick 中被调用
  window.changePage = function(newPage) {
    if (newPage >= 1 && newPage <= totalPages) {
      callback(newPage);
    }
  };
}

/** 打开新增弹窗 */
window.openAddUser = function() {
  document.getElementById('userModalTitle').textContent = '新增用户';
  document.getElementById('editUserId').value = '';
  document.getElementById('uName').value = '';
  document.getElementById('uName').disabled = false;
  document.getElementById('uRealName').value = '';
  document.getElementById('uPhone').value = '';
  document.getElementById('uPassword').value = '';
  document.getElementById('pwdRequired').style.display = 'inline';
  document.getElementById('uRole').value = '2'; // 默认医生
  document.getElementById('userModal').classList.add('show');
}

/** 打开编辑弹窗 */
window.openEditUser = function(id, username, realName, phone, roleId) {
  document.getElementById('userModalTitle').textContent = '编辑用户';
  document.getElementById('editUserId').value = id;
  document.getElementById('uName').value = username;
  document.getElementById('uName').disabled = true; // 用户名不能改
  document.getElementById('uRealName').value = realName;
  document.getElementById('uPhone').value = phone;
  document.getElementById('uPassword').value = ''; // 留空不改
  document.getElementById('pwdRequired').style.display = 'none';
  document.getElementById('uRole').value = roleId || '2';
  document.getElementById('userModal').classList.add('show');
}

/** 关闭弹窗 */
window.closeUserModal = function() {
  document.getElementById('userModal').classList.remove('show');
}

/** 保存用户 */
window.saveUser = async function() {
  const id = document.getElementById('editUserId').value;
  const username = document.getElementById('uName').value.trim();
  const realName = document.getElementById('uRealName').value.trim();
  const phone = document.getElementById('uPhone').value.trim();
  const password = document.getElementById('uPassword').value;
  const roleId = parseInt(document.getElementById('uRole').value);

  if (!username) return toast.error('请输入用户名');
  if (!realName) return toast.error('请输入真实姓名');
  if (!id && !password) return toast.error('新增用户时必须设置密码');

  const btn = document.getElementById('saveUserBtn');
  btn.disabled = true;
  btn.textContent = '保存中...';

  try {
    let res;
    if (id) {
      // Edit
      const body = { real_name: realName, role_id: roleId };
      if (phone) body.phone = phone;
      if (password) body.password = password;
      res = await api.put(`/users/${id}`, body);
    } else {
      // Add
      const body = { username, password, real_name: realName, role_id: roleId, phone };
      res = await api.post('/users', body);
    }

    if (res.code === 200) {
      toast.success(id ? '编辑成功' : '新增成功');
      closeUserModal();
      loadUsers(id ? currentPage : 1);
    } else {
      toast.error(res.msg || '保存失败');
    }
  } catch (err) {
    toast.error('网络请求失败');
  } finally {
    btn.disabled = false;
    btn.textContent = '保存';
  }
}

/** 删除用户 */
window.deleteUser = async function(id) {
  if (!confirm('确定要删除该用户吗？此操作不可恢复。')) return;

  try {
    const res = await api.del(`/users/${id}`);
    if (res.code === 200) {
      toast.success('删除成功');
      loadUsers();
    } else {
      toast.error(res.msg || '删除失败');
    }
  } catch (err) {
    toast.error('网络请求失败');
  }
}
