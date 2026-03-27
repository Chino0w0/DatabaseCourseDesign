/**
 * health.js — 健康档案页面逻辑
 */

let selectedResidentId = null;
let allDiseases = [];

// ── 初始化 ────────────────────────────────────────────────────
(function () {
  if (!api.requireAuth()) return;
  renderSidebar('health');
  loadDiseaseOptions();

  // 居民搜索去抖
  let searchTimer = null;
  document.getElementById('residentSearch').addEventListener('input', function () {
    clearTimeout(searchTimer);
    searchTimer = setTimeout(() => searchResidents(this.value.trim()), 300);
  });

  // 选择居民后自动查询
  document.getElementById('residentSelect').addEventListener('change', function () {
    selectedResidentId = parseInt(this.value) || null;
    if (selectedResidentId) loadHealthData();
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

// ── 选项卡切换 ────────────────────────────────────────────────
function switchTab(tabName) {
  document.querySelectorAll('.tab-btn').forEach(btn => btn.classList.remove('active'));
  document.querySelectorAll('.tab-content').forEach(el => el.classList.remove('active'));
  event.target.classList.add('active');
  document.getElementById(`tab-${tabName}`).classList.add('active');
}

// ── 加载全部健康数据 ─────────────────────────────────────────
async function loadHealthData() {
  if (!selectedResidentId) {
    toast.error('请先选择居民');
    return;
  }
  loadMeasurements();
  loadHealthRecord();
  loadResidentDiseases();
}

// ── 测量记录 ──────────────────────────────────────────────────
async function loadMeasurements() {
  const tbody = document.getElementById('measurementBody');
  tbody.innerHTML = '<tr><td colspan="10"><div class="loading"><div class="spinner"></div></div></td></tr>';
  const warningsArea = document.getElementById('warningsArea');
  warningsArea.innerHTML = '';

  const res = await api.get(`/health/measurements?resident_id=${selectedResidentId}&limit=100`);
  if (res.code !== 200 || !Array.isArray(res.data)) {
    tbody.innerHTML = '<tr><td colspan="10"><div class="empty-state"><div class="icon">❌</div><h3>加载失败</h3></div></td></tr>';
    return;
  }

  if (res.data.length === 0) {
    tbody.innerHTML = '<tr><td colspan="10"><div class="empty-state"><div class="icon">📭</div><h3>暂无测量记录</h3></div></td></tr>';
    return;
  }

  // 收集预警
  const allWarnings = [];
  res.data.forEach(m => {
    if (m.warning && m.warning_msg) {
      allWarnings.push(...m.warning_msg);
    }
  });

  if (allWarnings.length > 0) {
    warningsArea.innerHTML = `
      <div class="warning-card">
        <div class="warning-title">⚠ 健康预警 (${allWarnings.length} 条)</div>
        <ul>${allWarnings.map(w => `<li>${w}</li>`).join('')}</ul>
      </div>`;
  }

  tbody.innerHTML = res.data.map(m => `
    <tr>
      <td style="font-size:0.82rem;">${m.measured_at || '--'}</td>
      <td>${renderValue(m.systolic, 'mmHg')}</td>
      <td>${renderValue(m.diastolic, 'mmHg')}</td>
      <td>${renderValue(m.blood_sugar, 'mmol/L')}</td>
      <td>${renderValue(m.heart_rate, '次/分')}</td>
      <td>${renderValue(m.height, 'cm')}</td>
      <td>${renderValue(m.weight, 'kg')}</td>
      <td>${renderValue(m.bmi, '')}</td>
      <td>${m.measured_by || '--'}</td>
      <td>${m.warning ? '<span class="badge badge-danger">⚠ 异常</span>' : '<span class="badge badge-success">正常</span>'}</td>
    </tr>
  `).join('');
}

function renderValue(val, unit) {
  if (val === null || val === undefined) return '--';
  return `${val} <span style="color:var(--text-muted);font-size:0.75rem;">${unit}</span>`;
}

// ── 健康档案摘要 ─────────────────────────────────────────────
async function loadHealthRecord() {
  const el = document.getElementById('recordContent');
  el.innerHTML = '<div class="loading"><div class="spinner"></div></div>';

  const res = await api.get(`/health/records/${selectedResidentId}`);
  if (res.code !== 200 || !res.data) {
    el.innerHTML = '<div class="empty-state"><div class="icon">📭</div><h3>暂无健康档案</h3></div>';
    return;
  }
  const r = res.data;
  el.innerHTML = `
    <div class="detail-panel">
      <h3>📋 健康档案摘要</h3>
      <div class="detail-row"><div class="label">档案 ID</div><div class="value">${r.id}</div></div>
      <div class="detail-row"><div class="label">血型</div><div class="value">${r.blood_type || '未记录'}</div></div>
      <div class="detail-row"><div class="label">过敏史</div><div class="value">${r.allergy_history || '无'}</div></div>
      <div class="detail-row"><div class="label">家族病史</div><div class="value">${r.family_history || '无'}</div></div>
      <div class="detail-row"><div class="label">既往病史</div><div class="value">${r.past_medical_history || '无'}</div></div>
      <div class="detail-row"><div class="label">创建时间</div><div class="value">${r.created_at || '--'}</div></div>
      <div class="detail-row"><div class="label">更新时间</div><div class="value">${r.updated_at || '--'}</div></div>
    </div>`;
}

// ── 居民慢性病 ──────────────────────────────────────────────
async function loadResidentDiseases() {
  const el = document.getElementById('diseaseContent');
  el.innerHTML = '<div class="loading"><div class="spinner"></div></div>';

  const res = await api.get(`/residents/${selectedResidentId}/diseases`);
  if (res.code !== 200 || !Array.isArray(res.data)) {
    el.innerHTML = '<div class="empty-state"><div class="icon">❌</div><h3>加载失败</h3></div>';
    return;
  }

  if (res.data.length === 0) {
    el.innerHTML = '<div class="empty-state"><div class="icon">🎉</div><h3>未关联慢性病</h3></div>';
    return;
  }

  el.innerHTML = `
    <div class="table-wrapper">
      <table>
        <thead>
          <tr><th>疾病名称</th><th>疾病编码</th><th>分类</th><th>确诊日期</th><th>状态</th><th>备注</th></tr>
        </thead>
        <tbody>
          ${res.data.map(d => `
            <tr>
              <td><strong>${d.disease_name}</strong></td>
              <td style="font-family:monospace;">${d.disease_code || '--'}</td>
              <td>${d.category || '--'}</td>
              <td>${d.diagnosed_date || '--'}</td>
              <td><span class="badge ${d.status === '已痊愈' ? 'badge-success' : d.status === '控制中' ? 'badge-info' : 'badge-warning'}">${d.status || '治疗中'}</span></td>
              <td>${d.notes || '--'}</td>
            </tr>
          `).join('')}
        </tbody>
      </table>
    </div>`;
}

// ── 加载慢性病字典 ──────────────────────────────────────────
async function loadDiseaseOptions() {
  const res = await api.get('/diseases');
  if (res.code === 200 && Array.isArray(res.data)) {
    allDiseases = res.data;
    const select = document.getElementById('dDiseaseId');
    res.data.forEach(d => {
      select.innerHTML += `<option value="${d.id}">${d.disease_name}${d.category ? ' (' + d.category + ')' : ''}</option>`;
    });
  }
}

// ── 录入测量数据 ─────────────────────────────────────────────
function openMeasurementForm() {
  if (!selectedResidentId) { toast.error('请先选择居民'); return; }
  ['mSystolic', 'mDiastolic', 'mBloodSugar', 'mHeartRate', 'mHeight', 'mWeight', 'mNotes'].forEach(id => {
    document.getElementById(id).value = '';
  });
  document.getElementById('measurementModal').classList.add('show');
}

function closeMeasurementModal() {
  document.getElementById('measurementModal').classList.remove('show');
}

async function saveMeasurement() {
  const user = api.getUser();
  const body = {
    resident_id: selectedResidentId,
    measured_by_user_id: user?.user_id || null,
  };

  const systolic = document.getElementById('mSystolic').value;
  const diastolic = document.getElementById('mDiastolic').value;
  const bloodSugar = document.getElementById('mBloodSugar').value;
  const heartRate = document.getElementById('mHeartRate').value;
  const height = document.getElementById('mHeight').value;
  const weight = document.getElementById('mWeight').value;
  const notes = document.getElementById('mNotes').value.trim();

  if (systolic) body.systolic = parseInt(systolic);
  if (diastolic) body.diastolic = parseInt(diastolic);
  if (bloodSugar) body.blood_sugar = parseFloat(bloodSugar);
  if (heartRate) body.heart_rate = parseInt(heartRate);
  if (height) body.height = parseFloat(height);
  if (weight) body.weight = parseFloat(weight);
  if (notes) body.notes = notes;

  const res = await api.post('/health/measurements', body);
  if (res.code === 200) {
    closeMeasurementModal();
    if (res.data?.warning) {
      const msgs = res.data.warning_msg || [];
      toast.error('录入成功，但检测到预警：' + msgs.join('；'));
    } else {
      toast.success('录入测量数据成功');
    }
    loadMeasurements();
  } else {
    toast.error(res.msg || '录入失败');
  }
}

// ── 关联慢性病 ──────────────────────────────────────────────
function openAddDisease() {
  if (!selectedResidentId) { toast.error('请先选择居民'); return; }
  document.getElementById('dDiseaseId').value = '';
  document.getElementById('dDiagnosedDate').value = '';
  document.getElementById('dStatus').value = '治疗中';
  document.getElementById('dNotes').value = '';
  document.getElementById('diseaseModal').classList.add('show');
}

function closeDiseaseModal() {
  document.getElementById('diseaseModal').classList.remove('show');
}

async function saveDisease() {
  const diseaseId = parseInt(document.getElementById('dDiseaseId').value);
  if (!diseaseId) { toast.error('请选择慢性病类型'); return; }

  const body = {
    disease_id: diseaseId,
    diagnosed_date: document.getElementById('dDiagnosedDate').value || undefined,
    status: document.getElementById('dStatus').value,
    notes: document.getElementById('dNotes').value.trim() || undefined,
  };

  const res = await api.post(`/residents/${selectedResidentId}/diseases`, body);
  if (res.code === 200) {
    toast.success('关联慢性病成功');
    closeDiseaseModal();
    loadResidentDiseases();
  } else {
    toast.error(res.msg || '关联失败');
  }
}
