<template>
  <div class="login-container">
    <div class="login-box">
      <div class="login-header">
        <el-icon class="logo-icon" :size="40"><FirstAidKit /></el-icon>
        <h2 class="title">社区健康档案管理系统</h2>
        <p class="subtitle">Community Health Archive Management System</p>
      </div>

      <el-form
        ref="loginFormRef"
        :model="loginForm"
        :rules="loginRules"
        class="login-form"
        @keyup.enter="handleLogin"
      >
        <el-form-item prop="username">
          <el-input
            v-model="loginForm.username"
            placeholder="请输入管理员/医生/护士账号"
            prefix-icon="User"
            size="large"
            clearable
          />
        </el-form-item>

        <el-form-item prop="password">
          <el-input
            v-model="loginForm.password"
            type="password"
            placeholder="请输入密码"
            prefix-icon="Lock"
            size="large"
            show-password
          />
        </el-form-item>

        <el-button
          :loading="loading"
          type="primary"
          class="login-btn"
          size="large"
          @click="handleLogin"
        >
          登 录
        </el-button>
        
        <div class="tip-text">
          <span>* 基于 Vue 3 + Element Plus 的现代医疗化重构版本</span>
        </div>
      </el-form>
    </div>
  </div>
</template>

<script setup lang="ts">
import { ref, reactive } from 'vue'
import { useRouter } from 'vue-router'
import { useUserStore } from '@/store/user'
import { ElMessage } from 'element-plus'
import type { FormInstance, FormRules } from 'element-plus'
import request from '@/utils/request'
import { FirstAidKit } from '@element-plus/icons-vue'

const router = useRouter()
const userStore = useUserStore()

const loginFormRef = ref<FormInstance>()
const loading = ref(false)

const loginForm = reactive({
  username: '',
  password: ''
})

const loginRules = reactive<FormRules>({
  username: [{ required: true, trigger: 'blur', message: '请输入账户' }],
  password: [{ required: true, trigger: 'blur', message: '请输入密码' }]
})

const handleLogin = () => {
  loginFormRef.value?.validate(async (valid) => {
    if (valid) {
      loading.value = true
      try {
        const res: any = await request.post('/auth/login', loginForm)
        // 走到这里的必然是 200，因为我们在拦截器里直接把非200的响应 reject 了
        userStore.setAuth(res.data?.token || '', res.data)
        ElMessage.success(res.msg || '登录成功')
        router.push('/')
      } catch (err: any) {
        // HTTP 报错在 request 拦截器已通过友好的 Toast(轻提示) 处理，此时只需让按钮结束 Loading 即可
      } finally {
        loading.value = false
      }
    }
  })
}
</script>

<style lang="scss" scoped>
.login-container {
  display: flex;
  justify-content: center;
  align-items: center;
  height: 100vh;
  background-image: linear-gradient(135deg, #f5f7fa 0%, #c3cfe2 100%);
  
  // 现代感背景可添加医院/医疗相关矢量背景或更清新的渐变
  background-color: #e9eef3;
  
  .login-box {
    width: 420px;
    padding: 40px 35px;
    background: #fff;
    border-radius: 12px;
    box-shadow: 0 8px 24px rgba(0, 0, 0, 0.08);

    .login-header {
      text-align: center;
      margin-bottom: 35px;

      .logo-icon {
        color: #1890ff;
        margin-bottom: 10px;
      }

      .title {
        margin: 0;
        font-size: 24px;
        color: #303133;
        font-weight: 600;
        letter-spacing: 1px;
      }
      
      .subtitle {
        margin-top: 8px;
        font-size: 13px;
        color: #909399;
      }
    }

    .login-form {
      .login-btn {
        width: 100%;
        margin-top: 10px;
        font-size: 16px;
        border-radius: 6px;
      }
      
      .tip-text {
        margin-top: 20px;
        text-align: center;
        font-size: 12px;
        color: #c0c4cc;
      }
    }
  }
}
</style>
