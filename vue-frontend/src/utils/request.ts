import axios from 'axios'
import { ElMessage } from 'element-plus'
import { useUserStore } from '@/store/user'
import router from '@/router'

// 创建 axios 实例
const service = axios.create({
  baseURL: import.meta.env.VITE_APP_BASE_API || '/api/v1',
  timeout: 10000
})

// request 拦截器
service.interceptors.request.use(
  config => {
    const userStore = useUserStore()
    if (userStore.token) {
      config.headers['Authorization'] = 'Bearer ' + userStore.token
    }
    return config
  },
  error => {
    return Promise.reject(error)
  }
)

// response 拦截器
service.interceptors.response.use(
  response => {
    const res = response.data
    // 业务逻辑成功返回 (后端有时会把 code 写在外面)
    if (res.code === 200 || res.status === 'ok') {
      return res
    } else if (res.code === 401) {
      ElMessage.error(res.msg || '登录已失效，请重新登录')
      useUserStore().logout()
      if (router.currentRoute.value.path !== '/login') {
        router.push('/login')
      }
      return Promise.reject(new Error(res.msg || 'Error'))
    } else {
      ElMessage.error(res.msg || '请求错误')
      return Promise.reject(new Error(res.msg || 'Error'))
    }
  },
  error => {
    console.error('err:', error)
    if (error.response?.status === 401) {
      ElMessage.error(error.response?.data?.msg || '未授权访问，请重新登录')
      useUserStore().logout()
      if (router.currentRoute.value.path !== '/login') {
        router.push('/login')
      }
    } else {
      ElMessage.error(error.response?.data?.msg || '服务器开小差了，请确认网络连接或稍后再试~')
    }
    return Promise.reject(error)
  }
)

export default service
