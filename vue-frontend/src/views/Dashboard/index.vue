<template>
  <div class="dashboard-container">
    <el-row :gutter="20">
      <el-col :xs="24" :sm="12" :md="6">
        <StatCard title="社区居民总数" :value="statistics.residentsCount" bgClass="bg-blue">
          <template #icon><el-icon><User /></el-icon></template>
        </StatCard>
      </el-col>
      <el-col :xs="24" :sm="12" :md="6">
        <StatCard title="今日活动指派" value="暂无新任务" bgClass="bg-green">
          <template #icon><el-icon><RefreshRight /></el-icon></template>
          <template #value><span style="font-size: 20px;">系统已就绪</span></template>
        </StatCard>
      </el-col>
      <el-col :xs="24" :sm="12" :md="6">
        <StatCard title="活跃社区数" :value="statistics.hyperCount" bgClass="bg-orange">
          <template #icon><el-icon><FirstAidKit /></el-icon></template>
        </StatCard>
      </el-col>
      <el-col :xs="24" :sm="12" :md="6">
        <StatCard title="异常预警数" :value="statistics.diabeticCount" bgClass="bg-red">
          <template #icon><el-icon><Warning /></el-icon></template>
        </StatCard>
      </el-col>
    </el-row>

    <el-row :gutter="20" style="margin-top: 20px;">
      <el-col :xs="24" :lg="16">
        <el-card shadow="never" class="chart-card">
          <template #header>
            <div class="card-header-flex">
              <span>快捷操作向导</span>
            </div>
          </template>
          <div class="quick-actions">
            <div class="action-item" @click="$router.push('/residents')">
              <div class="icon-wrap" style="background: #e6f7ff; color: #1890ff;">
                <el-icon><DocumentAdd /></el-icon>
              </div>
              <span>新建档案</span>
            </div>
            <div class="action-item" @click="$router.push('/health')">
              <div class="icon-wrap" style="background: #f6ffed; color: #52c41a;">
                <el-icon><FirstAidKit /></el-icon>
              </div>
              <span>录入体检</span>
            </div>
            <div class="action-item" @click="$router.push('/visits')">
              <div class="icon-wrap" style="background: #fff0f6; color: #eb2f96;">
                <el-icon><EditPen /></el-icon>
              </div>
              <span>记录随访</span>
            </div>
            <div v-if="userStore.isAdmin" class="action-item" @click="$router.push('/system/users')">
              <div class="icon-wrap" style="background: #f9f0ff; color: #722ed1;">
                <el-icon><Setting /></el-icon>
              </div>
              <span>系统设置</span>
            </div>
          </div>
        </el-card>
      </el-col>
      <el-col :xs="24" :lg="8">
        <el-card shadow="never" class="timeline-card">
          <template #header>
            <span>系统最新动态 (实时居民建档追踪)</span>
          </template>
          <el-timeline>
            <el-timeline-item
              v-for="(activity, index) in activities"
              :key="index"
              :icon="activity.icon"
              :type="activity.type"
              :color="activity.color"
              :size="activity.size"
              :timestamp="activity.timestamp"
            >
              {{ activity.content }}
            </el-timeline-item>
            <el-empty v-if="activities.length === 0" description="暂无最新动态" :image-size="60" />
          </el-timeline>
        </el-card>
      </el-col>
    </el-row>
  </div>
</template>

<script setup lang="ts">
import { ref, reactive, onMounted } from 'vue'
import { useUserStore } from '@/store/user'
import {
  User, RefreshRight, Warning, FirstAidKit
} from '@element-plus/icons-vue'
import request from '@/utils/request'
import StatCard from '@/components/StatCard.vue'

const userStore = useUserStore()

const statistics = reactive({
  residentsCount: 0,
  visitsCount: 0,
  hyperCount: 0,
  diabeticCount: 0
})

const activities = ref<any[]>([])

const getDashboardStats = async () => {
  try {
    // 1. 获取真实居民总数
    const resRes: any = await request.get('/residents', { params: { page: 1, size: 5 } })
    if (resRes.code === 200) {
      statistics.residentsCount = resRes.data.total || 0
      
      // 提取最新的居民数据作为系统动态
      if (resRes.data.list && resRes.data.list.length > 0) {
        activities.value = resRes.data.list.map((r: any) => ({
          content: `新增档案: ${r.name} (${r.gender}) - ${r.community_name}`,
          timestamp: r.created_at || new Date().toISOString().slice(0, 16).replace('T', ' '),
          type: 'primary',
          icon: User,
          size: 'large'
        }))
      }
    }
    
    // 2. 获取社区总数
    const comRes: any = await request.get('/communities')
    if (comRes.code === 200) {
      statistics.hyperCount = comRes.data.list?.length || comRes.data.length || 0
    }
  } catch (err) {}
}

onMounted(() => {
  getDashboardStats()
})
</script>

<style lang="scss" scoped>
.dashboard-container {
  .data-card {
    color: #fff;
    border: none;
    
    &.bg-blue { background: linear-gradient(135deg, #36D1DC 0%, #5B86E5 100%); }
    &.bg-green { background: linear-gradient(135deg, #11998e 0%, #38ef7d 100%); }
    &.bg-purple { background: linear-gradient(135deg, #654ea3 0%, #eaafc8 100%); }
    &.bg-orange { background: linear-gradient(135deg, #ff9966 0%, #ff5e62 100%); }

    .card-header {
      display: flex;
      justify-content: space-between;
      align-items: center;
      font-size: 14px;
      opacity: 0.9;
    }
    
    .card-value {
      font-size: 32px;
      font-weight: bold;
      margin: 15px 0;
    }

    .card-footer {
      font-size: 12px;
      opacity: 0.8;
      span {
        font-weight: bold;
      }
    }
  }

  .chart-card, .timeline-card {
    min-height: 380px;
    margin-bottom: 20px;
    .card-header-flex {
      display: flex;
      justify-content: space-between;
      align-items: center;
    }
  }

  .quick-actions {
    display: flex;
    justify-content: space-around;
    padding: 30px 0;

    .action-item {
      display: flex;
      flex-direction: column;
      align-items: center;
      cursor: pointer;
      transition: transform 0.2s;

      &:hover {
        transform: translateY(-5px);
      }

      .icon-wrap {
        width: 60px;
        height: 60px;
        border-radius: 12px;
        display: flex;
        align-items: center;
        justify-content: center;
        font-size: 28px;
        margin-bottom: 12px;
      }

      span {
        font-size: 14px;
        color: #606266;
      }
    }
  }
}
</style>
