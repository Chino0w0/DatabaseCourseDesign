<template>
  <div class="app-container my-health-container">
    <el-page-header content="我的健康档案" @back="$router.push('/dashboard')" />

    <el-row :gutter="20" style="margin-top: 20px;">
      <el-col :xs="24" :lg="10">
        <el-card shadow="never" class="profile-card" v-loading="loading.profile">
          <template #header>
            <div class="card-header-flex">
              <span>档案摘要</span>
              <el-tag type="info">居民自助</el-tag>
            </div>
          </template>

          <template v-if="profile">
            <el-descriptions :column="1" border>
              <el-descriptions-item label="血型">{{ profile.blood_type || '未登记' }}</el-descriptions-item>
              <el-descriptions-item label="过敏史">{{ profile.allergy_history || '无' }}</el-descriptions-item>
              <el-descriptions-item label="家族病史">{{ profile.family_history || '无' }}</el-descriptions-item>
              <el-descriptions-item label="既往病史">{{ profile.past_medical_history || '无' }}</el-descriptions-item>
            </el-descriptions>
          </template>
          <el-empty v-else description="暂无健康档案" :image-size="80" />
        </el-card>
      </el-col>

      <el-col :xs="24" :lg="14">
        <el-card shadow="never" class="measure-card" v-loading="loading.measurements">
          <template #header>
            <div class="card-header-flex">
              <span>最近体检记录</span>
              <el-tag type="success">仅查看本人</el-tag>
            </div>
          </template>

          <el-table :data="measurements" stripe border style="width: 100%">
            <el-table-column prop="measured_at" label="测量时间" width="160" align="center" />
            <el-table-column label="血压" align="center">
              <template #default="{ row }">
                {{ row.systolic || '--' }} / {{ row.diastolic || '--' }}
              </template>
            </el-table-column>
            <el-table-column prop="blood_sugar" label="血糖" align="center" />
            <el-table-column prop="heart_rate" label="心率" align="center" />
            <el-table-column prop="measured_by" label="测量人" align="center" />
          </el-table>
          <el-empty v-if="!measurements.length && !loading.measurements" description="暂无体检记录" :image-size="70" />
        </el-card>
      </el-col>
    </el-row>

    <el-row :gutter="20" style="margin-top: 20px;">
      <el-col :xs="24" :lg="12">
        <el-card shadow="never" class="disease-card" v-loading="loading.diseases">
          <template #header>
            <div class="card-header-flex">
              <span>慢性病信息</span>
            </div>
          </template>

          <template v-if="diseases.length">
            <div class="disease-list">
              <el-tag v-for="item in diseases" :key="item.id" type="danger" effect="light" class="disease-tag">
                {{ item.disease_name }} · {{ item.status || '未标记状态' }}
              </el-tag>
            </div>
          </template>
          <el-empty v-else description="暂无慢性病信息" :image-size="70" />
        </el-card>
      </el-col>

      <el-col :xs="24" :lg="12">
        <el-card shadow="never" class="visit-card" v-loading="loading.visits">
          <template #header>
            <div class="card-header-flex">
              <span>最近随访记录</span>
            </div>
          </template>

          <el-timeline>
            <el-timeline-item
              v-for="item in visits"
              :key="item.id"
              :timestamp="item.visit_date"
              placement="top"
            >
              <div class="visit-item-title">{{ item.visit_type || '随访记录' }}</div>
              <div class="visit-item-content">{{ item.content || '无详细内容' }}</div>
              <div class="visit-item-footer">随访人：{{ item.visitor_name || '未记录' }}</div>
            </el-timeline-item>
          </el-timeline>
          <el-empty v-if="!visits.length && !loading.visits" description="暂无随访记录" :image-size="70" />
        </el-card>
      </el-col>
    </el-row>
  </div>
</template>

<script setup lang="ts">
import { onMounted, reactive, ref } from 'vue'
import request from '@/utils/request'

const profile = ref<any>(null)
const measurements = ref<any[]>([])
const diseases = ref<any[]>([])
const visits = ref<any[]>([])

const loading = reactive({
  profile: false,
  measurements: false,
  diseases: false,
  visits: false
})

const loadProfile = async () => {
  loading.profile = true
  try {
    const res: any = await request.get('/me/profile')
    if (res.code === 200) {
      profile.value = res.data
    }
  } finally {
    loading.profile = false
  }
}

const loadMeasurements = async () => {
  loading.measurements = true
  try {
    const res: any = await request.get('/me/measurements')
    if (res.code === 200) {
      measurements.value = res.data || []
    }
  } finally {
    loading.measurements = false
  }
}

const loadDiseases = async () => {
  loading.diseases = true
  try {
    const res: any = await request.get('/me/diseases')
    if (res.code === 200) {
      diseases.value = res.data || []
    }
  } finally {
    loading.diseases = false
  }
}

const loadVisits = async () => {
  loading.visits = true
  try {
    const res: any = await request.get('/me/visits')
    if (res.code === 200) {
      visits.value = res.data || []
    }
  } finally {
    loading.visits = false
  }
}

onMounted(() => {
  loadProfile()
  loadMeasurements()
  loadDiseases()
  loadVisits()
})
</script>

<style scoped>
.my-health-container {
  padding-bottom: 12px;
}

.card-header-flex {
  display: flex;
  justify-content: space-between;
  align-items: center;
}

.disease-list {
  display: flex;
  flex-wrap: wrap;
  gap: 10px;
}

.disease-tag {
  margin-bottom: 8px;
}

.visit-item-title {
  font-weight: 600;
  color: #303133;
}

.visit-item-content {
  margin-top: 6px;
  color: #606266;
  line-height: 1.6;
}

.visit-item-footer {
  margin-top: 6px;
  font-size: 12px;
  color: #909399;
}
</style>
