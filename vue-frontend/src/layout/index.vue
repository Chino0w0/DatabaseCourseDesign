<template>
  <el-container class="app-wrapper">
    <!-- Sidebar -->
    <el-aside width="240px" class="sidebar-container">
      <div class="logo">
        <el-icon :size="24" color="#fff"><FirstAidKit /></el-icon>
        <span>社区健康档案系统</span>
      </div>
      
      <el-menu
        :default-active="activeMenu"
        background-color="#001529"
        text-color="#a6adb4"
        active-text-color="#fff"
        router
        class="el-menu-vertical"
      >
        <el-menu-item v-for="item in visibleMenus" :key="item.path" :index="item.path">
          <el-icon><component :is="item.icon" /></el-icon>
          <template #title>{{ item.title }}</template>
        </el-menu-item>
      </el-menu>
    </el-aside>

    <el-container class="main-container">
      <!-- Navbar -->
      <el-header class="navbar">
        <div class="breadcrumb">
          <el-breadcrumb separator="/">
            <el-breadcrumb-item>{{ currentRouteTitle }}</el-breadcrumb-item>
          </el-breadcrumb>
        </div>
        <div class="right-menu">
          <el-dropdown trigger="click" @command="handleCommand">
            <div class="avatar-wrapper">
              <el-avatar :size="30" style="background-color:#409EFF">
                {{ userStore.userInfo?.username?.charAt(0).toUpperCase() }}
              </el-avatar>
              <span class="username">{{ userStore.userInfo?.username }}</span>
              <el-icon><CaretBottom /></el-icon>
            </div>
            <template #dropdown>
              <el-dropdown-menu>
                <el-dropdown-item disabled>
                  角色：{{ userStore.roleName || (userStore.isAdmin ? '系统管理员' : (userStore.isDoctor ? '医生' : (userStore.isNurse ? '护士' : '未分配'))) }}
                </el-dropdown-item>
                <el-dropdown-item divided command="logout">退出登录</el-dropdown-item>
              </el-dropdown-menu>
            </template>
          </el-dropdown>
        </div>
      </el-header>

      <!-- App Main -->
      <el-main class="app-main">
        <router-view v-slot="{ Component }">
          <transition name="fade-transform" mode="out-in">
            <component :is="Component" />
          </transition>
        </router-view>
      </el-main>
    </el-container>
  </el-container>
</template>

<script setup lang="ts">
import { computed } from 'vue'
import { useRouter, useRoute } from 'vue-router'
import { useUserStore } from '@/store/user'
import { ElMessage, ElMessageBox } from 'element-plus'
import { FirstAidKit, DataBoard, User, CopyDocument, Document, Setting, CaretBottom } from '@element-plus/icons-vue'

const router = useRouter()
const route = useRoute()
const userStore = useUserStore()

const ROLE_ADMIN = 'admin'
const ROLE_DOCTOR = 'doctor'
const ROLE_NURSE = 'nurse'
const ROLE_RESIDENT = 'resident'

const menuItems = [
  { path: '/dashboard', title: '数据概览', icon: DataBoard, roles: [ROLE_ADMIN, ROLE_DOCTOR, ROLE_NURSE] },
  { path: '/residents', title: '居民管理', icon: User, roles: [ROLE_ADMIN, ROLE_DOCTOR, ROLE_NURSE] },
  { path: '/communities', title: '社区管理', icon: Setting, roles: [ROLE_ADMIN, ROLE_DOCTOR] },
  { path: '/health', title: '健康档案', icon: CopyDocument, roles: [ROLE_ADMIN, ROLE_DOCTOR, ROLE_NURSE] },
  { path: '/visits', title: '随访管理', icon: Document, roles: [ROLE_ADMIN, ROLE_DOCTOR, ROLE_NURSE] },
  { path: '/my-health', title: '我的健康档案', icon: FirstAidKit, roles: [ROLE_RESIDENT] },
  { path: '/system/users', title: '系统管理', icon: Setting, roles: [ROLE_ADMIN] }
]

const hasAccess = (roles: string[]) => {
  return (
    (roles.includes(ROLE_ADMIN) && userStore.isAdmin) ||
    (roles.includes(ROLE_DOCTOR) && userStore.isDoctor) ||
    (roles.includes(ROLE_NURSE) && userStore.isNurse) ||
    (roles.includes(ROLE_RESIDENT) && userStore.isResident)
  )
}

const visibleMenus = computed(() => {
  return menuItems.filter(item => hasAccess(item.roles))
})

const activeMenu = computed(() => {
  return route.path
})

const currentRouteTitle = computed(() => {
  return route.meta.title || '工作台'
})

const handleCommand = (command: string) => {
  if (command === 'logout') {
    ElMessageBox.confirm('确定要退出登录吗?', '提示', {
      confirmButtonText: '确定',
      cancelButtonText: '取消',
      type: 'warning'
    }).then(() => {
      userStore.logout()
      router.push('/login')
      ElMessage.success('已安全退出')
    }).catch(() => {})
  }
}
</script>

<style lang="scss" scoped>
.app-wrapper {
  height: 100vh;
  width: 100%;
}

.sidebar-container {
  transition: width 0.28s;
  background-color: #001529;
  height: 100%;
  overflow: hidden;
  box-shadow: 2px 0 6px rgba(0,21,41,.35);
  display: flex;
  flex-direction: column;

  .logo {
    height: 60px;
    display: flex;
    align-items: center;
    padding-left: 20px;
    background: #002140;
    color: #fff;
    font-size: 16px;
    font-weight: 600;
    
    span {
      margin-left: 10px;
    }
  }

  .el-menu-vertical {
    border-right: none;
    flex: 1;
    overflow-y: auto;
    
    // 自定义选中高亮（更符合现代审美）
    .el-menu-item.is-active {
      background-color: #1890ff !important;
      color: #fff !important;
    }
    .el-menu-item:hover {
      color: #fff !important;
    }
  }
}

.main-container {
  display: flex;
  flex-direction: column;
  background-color: #f0f2f5;

  .navbar {
    height: 60px;
    background: #fff;
    box-shadow: 0 1px 4px rgba(0,21,41,.08);
    display: flex;
    align-items: center;
    justify-content: space-between;
    padding: 0 20px;
    z-index: 10;

    .breadcrumb {
      font-size: 14px;
    }

    .right-menu {
      display: flex;
      align-items: center;

      .avatar-wrapper {
        cursor: pointer;
        display: flex;
        align-items: center;
        gap: 8px;
        .username {
          font-size: 14px;
          color: #333;
        }
      }
    }
  }

  .app-main {
    flex: 1;
    overflow-y: auto;
    padding: 24px;
    position: relative;
  }
}

/* 路由转场动画 */
.fade-transform-leave-active,
.fade-transform-enter-active {
  transition: all 0.3s cubic-bezier(0.55, 0, 0.1, 1);
}
.fade-transform-enter-from {
  opacity: 0;
  transform: translateX(-15px);
}
.fade-transform-leave-to {
  opacity: 0;
  transform: translateX(15px);
}
</style>
