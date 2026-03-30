import axios from 'axios'
import { ElMessage } from 'element-plus'
import { useUserStore } from '@/store/user'
import router from '@/router'

let authMessageLock = false
let forbiddenMessageLock = false
let handlingUnauthorized = false

const showMessageOnce = (message: string, type: 'auth' | 'forbidden') => {
  const isAuth = type === 'auth'
  const locked = isAuth ? authMessageLock : forbiddenMessageLock
  if (locked) return

  if (isAuth) {
    authMessageLock = true
  } else {
    forbiddenMessageLock = true
  }

  ElMessage.error(message)
  window.setTimeout(() => {
    if (isAuth) {
      authMessageLock = false
    } else {
      forbiddenMessageLock = false
    }
  }, 2000)
}

const handleUnauthorized = (message: string) => {
  showMessageOnce(message, 'auth')
  if (handlingUnauthorized) return

  handlingUnauthorized = true
  const userStore = useUserStore()
  userStore.logout()

  if (router.currentRoute.value.path !== '/login') {
    void router.replace('/login')
  }

  window.setTimeout(() => {
    handlingUnauthorized = false
  }, 300)
}

const handleForbidden = (message: string) => {
  showMessageOnce(message, 'forbidden')
}

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
      handleUnauthorized(res.msg || '登录已失效，请重新登录')
      return Promise.reject(new Error(res.msg || 'Error'))
    } else if (res.code === 403) {
      handleForbidden(res.msg || '无权限访问该接口')
      return Promise.reject(new Error(res.msg || 'Forbidden'))
    } else {
      ElMessage.error(res.msg || '请求错误')
      return Promise.reject(new Error(res.msg || 'Error'))
    }
  },
  error => {
    console.error('err:', error)
    if (error.response?.status === 401) {
      handleUnauthorized(error.response?.data?.msg || '未授权访问，请重新登录')
    } else if (error.response?.status === 403) {
      handleForbidden(error.response?.data?.msg || '无权限访问该接口')
    } else {
      ElMessage.error(error.response?.data?.msg || '服务器开小差了，请确认网络连接或稍后再试~')
    }
    return Promise.reject(error)
  }
)

export default service
