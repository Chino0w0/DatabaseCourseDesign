import { defineStore } from 'pinia'

export const useUserStore = defineStore('user', {
  state: () => {
    return {
      token: localStorage.getItem('ch_token') || '',
      userInfo: JSON.parse(localStorage.getItem('ch_user') || 'null')
    }
  },
  getters: {
    isAdmin: (state) => state.userInfo?.role === 'admin' || state.userInfo?.role === '管理员' || state.userInfo?.role_id === 1 || state.userInfo?.username === 'admin'
  },
  actions: {
    setAuth(token: string, user: any) {
      this.token = token
      this.userInfo = user
      localStorage.setItem('ch_token', token)
      localStorage.setItem('ch_user', JSON.stringify(user))
    },
    logout() {
      this.token = ''
      this.userInfo = null
      localStorage.removeItem('ch_token')
      localStorage.removeItem('ch_user')
    }
  }
})
