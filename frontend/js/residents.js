/**
 * residents.js — 居民管理页面逻辑
 */

let currentPage = 1;
let pageSize = 10;
let communities = [];

// ── 初始化 ────────────────────────────────────────────────────
(function () {
  if (!api.requireAuth()) return;
  renderSidebar('residents');
  loadCommunities().then(() => loadResidents());
})();

// ── 加载社区列表 ──────────────────────────────────────────────
async function loadCommunities() {
  const res = await api.get('/communities');
  if (res.code === 200 && Array.isArray(res.data)) {
    communities = res.data;
    const filterEl = document.getElementById('filterCommunity');
    const formEl = document.getElementById('rCommunityId');
    communities.forEach(c => {
      filterEl.innerHTML += `<option value="${c.id}">${c.name}</option>`;
      formEl.innerHTML += `<option value="${c.id}">${c.name}</option>`;
    });
  }
}

// ── 加载居民列表 ──────────────────────────────────────────────
async function loadResidents() {
  const keyword = document.getElementById('searchKeyword').value.trim();
  const communityId = document.getElementById('filterCommunity').value;

  let url = `/residents?page=${currentPage}&size=${pageSize}`;
  if (keyword) url += `&keyword=${encodeURIComponent(keyword)}`;
  if (communityId && communityId !== '0') url += `&community_id=${communityId}`;

  const tbody = document.getElementById('residentTableBody');
  tbody.innerHTML = '<tr><td colspan="8"><div class="loading"><div class="spinner"></div></div></td></tr>';

  const res = await api.get(url);
  if (res.code !== 200 || !res.data) {
    tbody.innerHTML = '<tr><td colspan="8"><div class="empty-state"><div class="icon">❌</div><h3>加载失败</h3></div></td></tr>';
    return;
  }

  const { total, list } = res.data;
  if (!list || list.length === 0) {
    tbody.innerHTML = '<tr><td colspan="8"><div class="empty-state"><div class="icon">📭</div><h3>暂无居民数据</h3></div></td></tr>';
    renderPagination(0);
    return;
  }

  tbody.innerHTML = list.map(r => `
    <tr>
      <td>${r.id}</td>
      <td><strong>${r.name}</strong></td>
      <td><span class="badge ${r.gender === '男' ? 'badge-info' : 'badge-warning'}">${r.gender}</span></td>
      <td style="font-family:monospace;font-size:0.82rem;">${r.id_card}</td>
      <td>${r.phone || '--'}</td>
      <td>${r.community_name || '--'}</td>
      <td style="font-size:0.82rem;color:var(--text-muted);">${(r.created_at || '').substring(0, 10)}</td>
      <td>
        <div class="actions">
          <button class="btn btn-secondary btn-sm" onclick="viewResident(${r.id})">详情</button>
          <button class="btn btn-secondary btn-sm" onclick="editResident(${r.id})">编辑</button>
          <button class="btn btn-danger btn-sm" onclick="deleteResident(${r.id}, '${r.name}')">删除</button>
        </div>
      </td>
    </tr>
  `).join('');

  renderPagination(total);
}

// ── 分页渲染 ──────────────────────────────────────────────────
function renderPagination(total) {
  const totalPages = Math.ceil(total / pageSize) || 1;
  const el = document.getElementById('pagination');
  let html = '';
  html += `<button ${currentPage <= 1 ? 'disabled' : ''} onclick="goPage(${currentPage - 1})">‹ 上一页</button>`;
  html += `<span class="page-info">第 ${currentPage} / ${totalPages} 页（共 ${total} 条）</span>`;
  html += `<button ${currentPage >= totalPages ? 'disabled' : ''} onclick="goPage(${currentPage + 1})">下一页 ›</button>`;
  el.innerHTML = html;
}

function goPage(page) {
  currentPage = page;
  loadResidents();
}

// ── 搜索回车触发 ─────────────────────────────────────────────
document.getElementById('searchKeyword')?.addEventListener('keyup', (e) => {
  if (e.key === 'Enter') { currentPage = 1; loadResidents(); }
});

// ── 新增居民弹窗 ─────────────────────────────────────────────
function openAddResident() {
  document.getElementById('residentModalTitle').textContent = '新增居民';
  document.getElementById('editResidentId').value = '';
  document.getElementById('rName').value = '';
  document.getElementById('rGender').value = '男';
  document.getElementById('rIdCard').value = '';
  document.getElementById('rBirthDate').value = '';
  document.getElementById('rPhone').value = '';
  document.getElementById('rCommunityId').value = '0';
  document.getElementById('rAddress').value = '';
  document.getElementById('rEmergencyContact').value = '';
  document.getElementById('rEmergencyPhone').value = '';
  document.getElementById('residentModal').classList.add('show');
}

// ── 编辑居民弹窗 ─────────────────────────────────────────────
async function editResident(id) {
  const res = await api.get(`/residents/${id}`);
  if (res.code !== 200 || !res.data) {
    toast.error(res.msg || '获取居民信息失败');
    return;
  }
  const r = res.data;
  document.getElementById('residentModalTitle').textContent = '编辑居民';
  document.getElementById('editResidentId').value = r.id;
  document.getElementById('rName').value = r.name || '';
  document.getElementById('rGender').value = r.gender || '男';
  document.getElementById('rIdCard').value = r.id_card || '';
  document.getElementById('rBirthDate').value = r.birth_date || '';
  document.getElementById('rPhone').value = r.phone || '';
  document.getElementById('rCommunityId').value = r.community_id || 0;
  document.getElementById('rAddress').value = r.address || '';
  document.getElementById('rEmergencyContact').value = r.emergency_contact || '';
  document.getElementById('rEmergencyPhone').value = r.emergency_phone || '';
  document.getElementById('residentModal').classList.add('show');
}

// ── 保存居民 ─────────────────────────────────────────────────
async function saveResident() {
  const id = document.getElementById('editResidentId').value;
  const body = {
    name: document.getElementById('rName').value.trim(),
    gender: document.getElementById('rGender').value,
    id_card: document.getElementById('rIdCard').value.trim(),
    birth_date: document.getElementById('rBirthDate').value,
    phone: document.getElementById('rPhone').value.trim(),
    community_id: parseInt(document.getElementById('rCommunityId').value) || 0,
    address: document.getElementById('rAddress').value.trim(),
    emergency_contact: document.getElementById('rEmergencyContact').value.trim(),
    emergency_phone: document.getElementById('rEmergencyPhone').value.trim(),
  };

  if (!body.name || !body.gender || !body.id_card || !body.birth_date) {
    toast.error('请填写必填字段：姓名、性别、身份证号、出生日期');
    return;
  }

  let res;
  if (id) {
    res = await api.put(`/residents/${id}`, body);
  } else {
    res = await api.post('/residents', body);
  }

  if (res.code === 200) {
    toast.success(id ? '更新居民成功' : '新增居民成功');
    closeResidentModal();
    loadResidents();
  } else {
    toast.error(res.msg || '操作失败');
  }
}

function closeResidentModal() {
  document.getElementById('residentModal').classList.remove('show');
}

// ── 查看居民详情 ─────────────────────────────────────────────
async function viewResident(id) {
  const res = await api.get(`/residents/${id}`);
  if (res.code !== 200 || !res.data) {
    toast.error(res.msg || '获取居民信息失败');
    return;
  }
  const r = res.data;
  const el = document.getElementById('detailContent');

  // 同时加载慢性病
  const diseaseRes = await api.get(`/residents/${id}/diseases`);
  let diseaseHtml = '';
  if (diseaseRes.code === 200 && Array.isArray(diseaseRes.data) && diseaseRes.data.length > 0) {
    diseaseHtml = diseaseRes.data.map(d => `
      <div style="display:flex;align-items:center;gap:8px;padding:6px 0;">
        <span class="badge badge-warning">${d.status || '治疗中'}</span>
        <span>${d.disease_name}</span>
        <span style="color:var(--text-muted);font-size:0.8rem;">${d.diagnosed_date || ''}</span>
      </div>
    `).join('');
  } else {
    diseaseHtml = '<div style="color:var(--text-muted);padding:8px 0;">暂无慢性病记录</div>';
  }

  el.innerHTML = `
    <div class="detail-panel" style="margin-bottom:16px;">
      <h3>👤 基本信息</h3>
      <div class="detail-row"><div class="label">姓名</div><div class="value">${r.name}</div></div>
      <div class="detail-row"><div class="label">性别</div><div class="value">${r.gender}</div></div>
      <div class="detail-row"><div class="label">身份证号</div><div class="value" style="font-family:monospace;">${r.id_card}</div></div>
      <div class="detail-row"><div class="label">出生日期</div><div class="value">${r.birth_date}</div></div>
      <div class="detail-row"><div class="label">联系电话</div><div class="value">${r.phone || '--'}</div></div>
      <div class="detail-row"><div class="label">所属社区</div><div class="value">${r.community_name || '--'}</div></div>
      <div class="detail-row"><div class="label">详细地址</div><div class="value">${r.address || '--'}</div></div>
      <div class="detail-row"><div class="label">紧急联系人</div><div class="value">${r.emergency_contact || '--'} ${r.emergency_phone || ''}</div></div>
      <div class="detail-row"><div class="label">建档时间</div><div class="value">${r.created_at || '--'}</div></div>
    </div>
    <div class="detail-panel" style="margin-bottom:0;">
      <h3>🩺 慢性病记录</h3>
      ${diseaseHtml}
    </div>
  `;
  document.getElementById('detailModal').classList.add('show');
}

function closeDetailModal() {
  document.getElementById('detailModal').classList.remove('show');
}

// ── 删除居民 ─────────────────────────────────────────────────
async function deleteResident(id, name) {
  if (!confirm(`确定要删除居民「${name}」吗？此操作不可撤销。`)) return;
  const res = await api.del(`/residents/${id}`);
  if (res.code === 200) {
    toast.success(`已删除居民「${name}」`);
    loadResidents();
  } else {
    toast.error(res.msg || '删除失败');
  }
}

// ── 新增社区 ─────────────────────────────────────────────────
function openAddCommunity() {
  document.getElementById('cName').value = '';
  document.getElementById('cAddress').value = '';
  document.getElementById('cPhone').value = '';
  document.getElementById('communityModal').classList.add('show');
}

function closeCommunityModal() {
  document.getElementById('communityModal').classList.remove('show');
}

async function saveCommunity() {
  const name = document.getElementById('cName').value.trim();
  if (!name) { toast.error('社区名称不能为空'); return; }

  const res = await api.post('/communities', {
    name,
    address: document.getElementById('cAddress').value.trim(),
    contact_phone: document.getElementById('cPhone').value.trim(),
  });

  if (res.code === 200) {
    toast.success('新增社区成功');
    closeCommunityModal();
    // 刷新社区下拉和列表
    const filterEl = document.getElementById('filterCommunity');
    const formEl = document.getElementById('rCommunityId');
    if (res.data?.id) {
      const opt1 = new Option(name, res.data.id);
      const opt2 = new Option(name, res.data.id);
      filterEl.add(opt1);
      formEl.add(opt2);
    }
  } else {
    toast.error(res.msg || '新增社区失败');
  }
}
