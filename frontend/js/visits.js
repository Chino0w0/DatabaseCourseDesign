/**
 * visits.js — 随访管理页面逻辑
 */

let selectedResidentId = null;

// ── 初始化 ────────────────────────────────────────────────────
(function () {
  if (!api.requireAuth()) return;
  renderSidebar('visits');

  // 预填今天日期
  const today = new Date().toISOString().split('T')[0];
  document.getElementById('vVisitDate').value = today;

  // 居民搜索去抖
  let searchTimer = null;
  document.getElementById('residentSearch').addEventListener('input', function () {
    clearTimeout(searchTimer);
    searchTimer = setTimeout(() => searchResidents(this.value.trim()), 300);
  });

  // 选择居民后自动查询
  document.getElementById('residentSelect').addEventListener('change', function () {
    selectedResidentId = parseInt(this.value) || null;
    if (selectedResidentId) loadVisits();
  });
})();

// ── 搜索居民 ──────────────────────────────────────────────────
async function searchResidents(keyword) {
  if (!keyword) return;
  const res = await api.get(`/residents?page=1&size=20&keyword=${encodeURIComponent(keyword)}`);
  const select = document.getElementById('residentSelect');
  select.innerHTML = '<option value="">请选择居民</option>';
  if (res.code === 200 && res.data?.list) {
    res.data.list.forEach(r => {
      select.innerHTML += `<option value="${r.id}">${r.name} (${r.id_card.substring(0, 6)}...${r.id_card.substring(14)})</option>`;
    });
  }
}

// ── 加载随访记录 ──────────────────────────────────────────────
async function loadVisits() {
  if (!selectedResidentId) {
    toast.error('请先选择居民');
    return;
  }

  const tbody = document.getElementById('visitTableBody');
  tbody.innerHTML = '<tr><td colspan="7"><div class="loading"><div class="spinner"></div></div></td></tr>';

  const res = await api.get(`/visits?resident_id=${selectedResidentId}`);
  if (res.code !== 200 || !Array.isArray(res.data)) {
    tbody.innerHTML = '<tr><td colspan="7"><div class="empty-state"><div class="icon">❌</div><h3>加载失败</h3></div></td></tr>';
    return;
  }

  if (res.data.length === 0) {
    tbody.innerHTML = '<tr><td colspan="7"><div class="empty-state"><div class="icon">📭</div><h3>暂无随访记录</h3></div></td></tr>';
    return;
  }

  tbody.innerHTML = res.data.map(v => {
    const typeClass = v.visit_type === '上门随访' ? 'badge-info' :
                      v.visit_type === '电话随访' ? 'badge-warning' : 'badge-success';
    return `
      <tr>
        <td>${v.id}</td>
        <td>${v.visit_date || '--'}</td>
        <td><span class="badge ${typeClass}">${v.visit_type || '--'}</span></td>
        <td>${v.visitor_name || '--'}</td>
        <td style="max-width:260px;overflow:hidden;text-overflow:ellipsis;white-space:nowrap;">${v.content || '--'}</td>
        <td>${v.next_visit_date ? `<span style="color:var(--info);">${v.next_visit_date}</span>` : '--'}</td>
        <td style="font-size:0.82rem;color:var(--text-muted);">${v.created_at || '--'}</td>
      </tr>
    `;
  }).join('');
}

// ── 新增随访 ──────────────────────────────────────────────────
function openAddVisit() {
  if (!selectedResidentId) { toast.error('请先选择居民'); return; }
  document.getElementById('vContent').value = '';
  document.getElementById('vNextVisitDate').value = '';
  document.getElementById('visitModal').classList.add('show');
}

function closeVisitModal() {
  document.getElementById('visitModal').classList.remove('show');
}

async function saveVisit() {
  const user = api.getUser();
  const visitType = document.getElementById('vVisitType').value;
  const visitDate = document.getElementById('vVisitDate').value;
  const content = document.getElementById('vContent').value.trim();
  const nextVisitDate = document.getElementById('vNextVisitDate').value;

  if (!visitType || !visitDate) {
    toast.error('请填写随访方式和随访日期');
    return;
  }

  const body = {
    resident_id: selectedResidentId,
    visitor_user_id: user?.user_id || 1,
    visit_type: visitType,
    visit_date: visitDate,
  };

  if (content) body.content = content;
  if (nextVisitDate) body.next_visit_date = nextVisitDate;

  const res = await api.post('/visits', body);
  if (res.code === 200) {
    toast.success('新增随访记录成功');
    closeVisitModal();
    loadVisits();
  } else {
    toast.error(res.msg || '新增失败');
  }
}
