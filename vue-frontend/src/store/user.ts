import { defineStore } from 'pinia'

export const useUserStore = defineStore('user', {
  state: () => {
    return {
      token: localStorage.getItem('ch_token') || '',
      userInfo: JSON.parse(localStorage.getItem('ch_user') || 'null')
    }
  },
  getters: {
    roleId: (state) => Number(state.userInfo?.role_id || 0),
    roleName: (state) => state.userInfo?.role || state.userInfo?.roleName || state.userInfo?.role_name || '',
    residentId: (state) => Number(state.userInfo?.resident_id || 0),
    isAdmin(): boolean {
      return this.roleId === 1 || this.roleName === '管理员' || this.userInfo?.username === 'admin'
    },
    isDoctor(): boolean {
      return this.roleId === 2 || this.roleName === '医生'
    },
    isNurse(): boolean {
      return this.roleId === 3 || this.roleName === '护士'
    },
    isResident(): boolean {
      return this.roleId === 4 || this.roleName === '居民'
    },
    canWriteResident(): boolean {
      return this.isAdmin || this.isDoctor
    },
    canDeleteResident(): boolean {
      return this.isAdmin
    },
    canManageCommunity(): boolean {
      return this.isAdmin || this.isDoctor
    },
    canWriteMeasurement(): boolean {
      return this.isAdmin || this.isDoctor || this.isNurse
    },
    canWriteVisit(): boolean {
      return this.isAdmin || this.isDoctor
    },
    canWriteDisease(): boolean {
      return this.isAdmin || this.isDoctor
    }
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
