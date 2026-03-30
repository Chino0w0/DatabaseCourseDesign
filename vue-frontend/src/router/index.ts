import { createRouter, createWebHashHistory, RouteRecordRaw } from 'vue-router'
import { useUserStore } from '@/store/user'
import Layout from '@/layout/index.vue'

const ROLE_ADMIN = 'admin'
const ROLE_DOCTOR = 'doctor'
const ROLE_NURSE = 'nurse'
const ROLE_RESIDENT = 'resident'

const canAccessRoles = (roles: string[], userStore: ReturnType<typeof useUserStore>) => {
  if (!roles || roles.length === 0) return true
  return (
    (roles.includes(ROLE_ADMIN) && userStore.isAdmin) ||
    (roles.includes(ROLE_DOCTOR) && userStore.isDoctor) ||
    (roles.includes(ROLE_NURSE) && userStore.isNurse) ||
    (roles.includes(ROLE_RESIDENT) && userStore.isResident)
  )
}

const getDefaultHomePath = (userStore: ReturnType<typeof useUserStore>) => {
  if (userStore.isResident) return '/my-health'
  return '/dashboard'
}

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
        meta: { title: '数据概览', icon: 'DataBoard', roles: [ROLE_ADMIN, ROLE_DOCTOR, ROLE_NURSE] }
      },
      {
        path: 'residents',
        name: 'Residents',
        component: () => import('@/views/Resident/index.vue'),
        meta: { title: '居民管理', icon: 'User', roles: [ROLE_ADMIN, ROLE_DOCTOR, ROLE_NURSE] }
      },
      {
        path: 'health',
        name: 'Health',
        component: () => import('@/views/Health/index.vue'),
        meta: { title: '健康档案', icon: 'FirstAidKit', roles: [ROLE_ADMIN, ROLE_DOCTOR, ROLE_NURSE] }
      },
      {
        path: 'visits',
        name: 'Visits',
        component: () => import('@/views/Visit/index.vue'),
        meta: { title: '随访管理', icon: 'Document', roles: [ROLE_ADMIN, ROLE_DOCTOR, ROLE_NURSE] }
      },
      {
        path: 'my-health',
        name: 'MyHealth',
        component: () => import('@/views/MyHealth/index.vue'),
        meta: { title: '我的健康档案', icon: 'FirstAidKit', roles: [ROLE_RESIDENT] }
      },
      {
        path: 'communities',
        name: 'Communities',
        component: () => import('@/views/Community/index.vue'),
        meta: { title: '社区管理', icon: 'OfficeBuilding', roles: [ROLE_ADMIN, ROLE_DOCTOR] }
      }
    ]
  },
  {
    path: '/system',
    component: Layout,
    name: 'System',
    meta: { title: '系统管理', icon: 'Setting', roles: [ROLE_ADMIN] },
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
    if (hasToken) return next(getDefaultHomePath(userStore))
    return next()
  }

  if (!hasToken) {
    return next('/login')
  }

  if (to.path === '/') {
    return next(getDefaultHomePath(userStore))
  }

  const routeRoles = to.matched.flatMap(record => {
    const roles = record.meta.roles
    return Array.isArray(roles) ? roles : []
  })

  if (routeRoles.length > 0 && !canAccessRoles(routeRoles, userStore)) {
    return next(getDefaultHomePath(userStore))
  }

  next()
})

export default router
