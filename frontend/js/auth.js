/**
 * auth.js — 登录认证逻辑
 */

(function () {
  // 如果已登录，直接跳转仪表盘
  if (api.getToken()) {
    window.location.href = '/pages/dashboard.html';
    return;
  }

  const form = document.getElementById('loginForm');
  const errorEl = document.getElementById('loginError');
  const loginBtn = document.getElementById('loginBtn');

  form.addEventListener('submit', async (e) => {
    e.preventDefault();
    errorEl.textContent = '';

    const username = document.getElementById('username').value.trim();
    const password = document.getElementById('password').value;

    if (!username || !password) {
      errorEl.textContent = '请输入用户名和密码';
      return;
    }

    loginBtn.disabled = true;
    loginBtn.textContent = '登录中...';

    try {
      const res = await api.post('/auth/login', { username, password });

      if (res.code === 200 && res.data && res.data.token) {
        api.saveAuth(res.data.token, {
          user_id: res.data.user_id,
          username: res.data.username,
          role: res.data.role,
        });
        window.location.href = '/pages/dashboard.html';
      } else if (res.code === 200) {
        errorEl.textContent = '登录响应缺少令牌 token，请先检查后端登录接口';
      } else {
        errorEl.textContent = res.msg || '登录失败';
      }
    } catch (err) {
      errorEl.textContent = '网络错误，请检查后端服务';
    } finally {
      loginBtn.disabled = false;
      loginBtn.textContent = '登 录';
    }
  });
})();
