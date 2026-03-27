import { createRouter, createWebHashHistory, RouteRecordRaw } from 'vue-router'
import { useUserStore } from '@/store/user'
import Layout from '@/layout/index.vue'

const routes: Array<RouteRecordRaw> = [
  {
    path: '/login',
    name: 'Login',
    component: () => import('@/views/Login/index.vue'),
    meta: { hidden: true }
  },
  {
    path: '/',
    component: Layout,
    redirect: '/dashboard',
    children: [
      {
        path: 'dashboard',
        name: 'Dashboard',
        component: () => import('@/views/Dashboard/index.vue'),
        meta: { title: '数据概览', icon: 'DataBoard' }
      },
      {
        path: 'residents',
        name: 'Residents',
        component: () => import('@/views/Resident/index.vue'),
        meta: { title: '居民管理', icon: 'User' }
      },
      {
        path: 'health',
        name: 'Health',
        component: () => import('@/views/Health/index.vue'),
        meta: { title: '健康档案', icon: 'FirstAidKit' }
      },
      {
        path: 'visits',
        name: 'Visits',
        component: () => import('@/views/Visit/index.vue'),
        meta: { title: '随访管理', icon: 'Document' }
      }
    ]
  },
  {
    path: '/system',
    component: Layout,
    name: 'System',
    meta: { title: '系统管理', icon: 'Setting', role: 'admin' },
    children: [
      {
        path: 'users',
        name: 'Users',
        component: () => import('@/views/System/Users.vue'),
        meta: { title: '用户管理', icon: 'UserFilled' }
      }
    ]
  },
  // Catch all
  {
    path: '/:pathMatch(.*)*',
    redirect: '/login'
  }
]

const router = createRouter({
  history: createWebHashHistory(),
  routes
})

// 路由守卫
router.beforeEach((to, _from, next) => {
  const userStore = useUserStore()
  const hasToken = userStore.token

  if (to.path === '/login') {
    if (hasToken) return next('/')
    return next()
  }

  if (!hasToken) {
    return next('/login')
  }

  // 检查管理员权限路由拦截
  const requireAdmin = to.matched.some(record => record.meta.role === 'admin')
  if (requireAdmin && !userStore.isAdmin) {
    return next('/')
  }

  next()
})

export default router
