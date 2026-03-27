/**
 * api.js — 统一 API 请求封装
 *
 * 所有页面通过 api.get / api.post / api.put / api.del 调用后端 API，
 * 自动携带 Authorization: Bearer <token>，统一处理 401 自动跳转登录。
 */

const API_BASE = '/api/v1';

const api = {
  /**
   * 获取保存的 token
   */
  getToken() {
    return localStorage.getItem('ch_token') || '';
  },

  /**
   * 获取保存的用户信息
   */
  getUser() {
    try {
      return JSON.parse(localStorage.getItem('ch_user') || 'null');
    } catch { return null; }
  },

  /**
   * 保存登录凭据
   */
  saveAuth(token, user) {
    localStorage.setItem('ch_token', token);
    localStorage.setItem('ch_user', JSON.stringify(user));
  },

  /**
   * 清除登录凭据
   */
  clearAuth() {
    localStorage.removeItem('ch_token');
    localStorage.removeItem('ch_user');
  },

  /**
   * 检查是否已登录；未登录则跳转
   */
  requireAuth() {
    if (!this.getToken()) {
      window.location.href = '/pages/login.html';
      return false;
    }
    return true;
  },

  /**
   * 通用请求方法
   */
  async request(method, path, body = null) {
    const headers = {
      'Content-Type': 'application/json',
    };
    const token = this.getToken();
    if (token) {
      headers['Authorization'] = 'Bearer ' + token;
    }

    const opts = { method, headers };
    if (body !== null && method !== 'GET') {
      opts.body = JSON.stringify(body);
    }

    try {
      const resp = await fetch(API_BASE + path, opts);
      const data = await resp.json();

      // 401 未认证 → 跳转登录
      if (data.code === 401) {
        this.clearAuth();
        window.location.href = '/pages/login.html';
        return data;
      }

      return data;
    } catch (err) {
      console.error('[API] 请求失败:', err);
      return { code: 500, msg: '网络请求失败，请检查后端服务', data: null };
    }
  },

  // 便捷方法
  get(path) { return this.request('GET', path); },
  post(path, body) { return this.request('POST', path, body); },
  put(path, body) { return this.request('PUT', path, body); },
  del(path) { return this.request('DELETE', path); },
};

// ── Toast 提示系统 ────────────────────────────────────────────
const toast = {
  _container: null,

  _getContainer() {
    if (!this._container) {
      this._container = document.createElement('div');
      this._container.className = 'toast-container';
      document.body.appendChild(this._container);
    }
    return this._container;
  },

  show(msg, type = 'info', duration = 3000) {
    const el = document.createElement('div');
    el.className = `toast toast-${type}`;
    el.textContent = msg;
    el.addEventListener('click', () => el.remove());
    this._getContainer().appendChild(el);
    setTimeout(() => {
      if (el.parentNode) el.remove();
    }, duration);
  },

  success(msg) { this.show(msg, 'success'); },
  error(msg) { this.show(msg, 'error', 5000); },
  info(msg) { this.show(msg, 'info'); },
};

// ── 侧边栏渲染 ──────────────────────────────────────────────
function renderSidebar(activePage) {
  const user = api.getUser();
  const initial = user?.username?.charAt(0)?.toUpperCase() || '?';

  const nav = [
    { id: 'dashboard', label: '数据概览', icon: '📊', href: '/pages/dashboard.html' },
    { id: 'residents', label: '居民管理', icon: '👥', href: '/pages/residents.html' },
    { id: 'health',    label: '健康档案', icon: '🏥', href: '/pages/health.html' },
    { id: 'visits',    label: '随访管理', icon: '📋', href: '/pages/visits.html' },
  ];

  const sidebar = document.getElementById('sidebar');
  if (!sidebar) return;

  sidebar.innerHTML = `
    <div class="sidebar-brand">
      <h2>社区健康档案</h2>
      <span>管理系统 v1.0</span>
    </div>
    <ul class="sidebar-nav">
      ${nav.map(n => `
        <li>
          <a href="${n.href}" class="${n.id === activePage ? 'active' : ''}">
            <span class="nav-icon">${n.icon}</span>
            ${n.label}
          </a>
        </li>
      `).join('')}
    </ul>
    <div class="sidebar-bottom">
      <div class="sidebar-user">
        <div class="avatar">${initial}</div>
        <div class="user-info">
          <div class="name">${user?.username || '未知'}</div>
          <div class="role">${user?.role || '未知角色'}</div>
        </div>
        <button class="btn-logout" onclick="handleLogout()" title="退出登录">退出</button>
      </div>
    </div>
  `;
}

function handleLogout() {
  api.clearAuth();
  window.location.href = '/pages/login.html';
}
