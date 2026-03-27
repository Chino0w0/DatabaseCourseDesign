<template>
  <div class="app-container">
    <div class="filter-container" style="display:flex; gap:15px; align-items:center;">
      <el-select
        v-model="selectedResidentId"
        filterable
        remote
        reserve-keyword
        placeholder="请输入居民姓名搜索"
        :remote-method="searchResidents"
        :loading="searchLoading"
        style="width: 300px"
        @change="handleResidentSelect"
      >
        <el-option
          v-for="item in residentOptions"
          :key="item.id"
          :label="`${item.name} (${item.id_card.slice(-4)})`"
          :value="item.id"
        />
      </el-select>

      <el-button type="primary" icon="Search" :disabled="!selectedResidentId" @click="getMeasurements">
        查询体检记录
      </el-button>
      
      <el-button type="success" icon="FirstAidKit" :disabled="!selectedResidentId" @click="handleCreate">
        录入体检数据
      </el-button>
    </div>

    <!-- 居民基本档案摘要卡片 -->
    <el-card v-if="healthProfile" class="box-card" style="margin-top: 20px; margin-bottom: 20px;">
      <template #header>
        <div class="card-header">
          <span>健康档案摘要</span>
          <el-tag type="info">血型: {{ healthProfile.blood_type || '未登记' }}</el-tag>
        </div>
      </template>
      <el-descriptions :column="3" border>
        <el-descriptions-item label="过敏史">{{ healthProfile.allergy_history || '无' }}</el-descriptions-item>
        <el-descriptions-item label="家族病史">{{ healthProfile.family_history || '无' }}</el-descriptions-item>
        <el-descriptions-item label="既往病史">{{ healthProfile.past_medical_history || '无' }}</el-descriptions-item>
      </el-descriptions>
    </el-card>

    <el-table
      v-loading="listLoading"
      :data="measurements"
      border
      stripe
      style="width: 100%"
      height="450"
    >
      <el-table-column prop="measured_at" label="测量时间" width="180" align="center" />
      <el-table-column label="血压 (mmHg)" align="center">
        <template #default="{ row }">
          <span :class="{'text-danger': row.systolic > 140 || row.diastolic > 90}">
            {{ row.systolic }} / {{ row.diastolic }}
          </span>
        </template>
      </el-table-column>
      <el-table-column prop="blood_sugar" label="血糖 (mmol/L)" align="center" />
      <el-table-column prop="heart_rate" label="心率 (次/分)" align="center" />
      <el-table-column label="身高/体重" align="center">
        <template #default="{ row }">
          {{ row.height }}cm / {{ row.weight }}kg
        </template>
      </el-table-column>
      <el-table-column prop="bmi" label="BMI" align="center">
        <template #default="{ row }">
          <el-tag :type="getBmiType(row.bmi)">{{ row.bmi }}</el-tag>
        </template>
      </el-table-column>
      <el-table-column prop="measured_by" label="测量人" align="center" />
      <el-table-column label="异常预警" width="200" show-overflow-tooltip>
        <template #default="{ row }">
          <el-tag v-if="row.warning" type="danger" size="small">警告</el-tag>
          <span v-if="row.warning" style="color:red; margin-left:5px; font-size:12px;">{{ row.warning_msg?.join('; ') }}</span>
          <span v-else class="text-success" style="font-size:12px;">正常</span>
        </template>
      </el-table-column>
    </el-table>

    <!-- 录入体检数据弹窗 -->
    <el-dialog title="新增体检数据" v-model="dialogFormVisible" width="500px">
      <el-form ref="dataFormRef" :rules="rules" :model="temp" label-position="right" label-width="120px">
        <el-form-item label="收缩压(高压)" prop="systolic">
          <el-input-number v-model="temp.systolic" :min="40" :max="300" />
        </el-form-item>
        <el-form-item label="舒张压(低压)" prop="diastolic">
          <el-input-number v-model="temp.diastolic" :min="30" :max="200" />
        </el-form-item>
        <el-form-item label="空腹血糖" prop="blood_sugar">
          <el-input-number v-model="temp.blood_sugar" :min="1" :max="30" :precision="1" :step="0.1" />
        </el-form-item>
        <el-form-item label="心率" prop="heart_rate">
          <el-input-number v-model="temp.heart_rate" :min="30" :max="200" />
        </el-form-item>
      </el-form>
      <template #footer>
        <div class="dialog-footer">
          <el-button @click="dialogFormVisible = false">取消</el-button>
          <el-button type="primary" :loading="submitLoading" @click="createData">确 定</el-button>
        </div>
      </template>
    </el-dialog>
  </div>
</template>

<script setup lang="ts">
import { ref, reactive } from 'vue'
import request from '@/utils/request'
import { ElMessage } from 'element-plus'
import type { FormInstance, FormRules } from 'element-plus'
import { useUserStore } from '@/store/user'

const userStore = useUserStore()

const searchLoading = ref(false)
const residentOptions = ref<any[]>([])
const selectedResidentId = ref<number | undefined>(undefined)

const listLoading = ref(false)
const measurements = ref<any[]>([])
const healthProfile = ref<any>(null)

// 搜索居民
const searchResidents = async (query: string) => {
  if (query) {
    searchLoading.value = true
    try {
      const res: any = await request.get('/residents', { params: { keyword: query, page: 1, size: 20 } })
      if (res.code === 200) {
        residentOptions.value = res.data.list || []
      }
    } finally {
      searchLoading.value = false
    }
  } else {
    residentOptions.value = []
  }
}

const handleResidentSelect = () => {
  if (selectedResidentId.value) {
    getMeasurements()
    getHealthProfile()
  } else {
    measurements.value = []
    healthProfile.value = null
  }
}

const getHealthProfile = async () => {
  try {
    const res: any = await request.get(`/health/records/${selectedResidentId.value}`)
    if (res.code === 200) {
      healthProfile.value = res.data
    } else {
      healthProfile.value = null
    }
  } catch (err) {
    healthProfile.value = null
  }
}

const getMeasurements = async () => {
  if (!selectedResidentId.value) return
  listLoading.value = true
  try {
    const res: any = await request.get('/health/measurements', { params: { resident_id: selectedResidentId.value } })
    if (res.code === 200) {
      measurements.value = res.data || []
    }
  } finally {
    listLoading.value = false
  }
}

const getBmiType = (bmi: number) => {
  if (!bmi) return 'info'
  if (bmi < 18.5) return 'warning'
  if (bmi < 24) return 'success'
  if (bmi < 28) return 'warning'
  return 'danger'
}

// 录入数据弹窗逻辑
const dialogFormVisible = ref(false)
const dataFormRef = ref<FormInstance>()
const submitLoading = ref(false)

const temp = reactive({
  systolic: 120,
  diastolic: 80,
  blood_sugar: 5.5,
  heart_rate: 75
})

const rules = reactive<FormRules>({
  systolic: [{ required: true, message: '必填', trigger: 'blur' }],
  diastolic: [{ required: true, message: '必填', trigger: 'blur' }]
})

const handleCreate = () => {
  temp.systolic = 120
  temp.diastolic = 80
  temp.blood_sugar = 5.5
  temp.heart_rate = 75
  dialogFormVisible.value = true
  setTimeout(() => dataFormRef.value?.clearValidate(), 0)
}

const createData = () => {
  dataFormRef.value?.validate(async (valid) => {
    if (valid) {
      submitLoading.value = true
      try {
        const payload = {
          ...temp,
          resident_id: selectedResidentId.value,
          measured_by_user_id: userStore.userInfo?.id || userStore.userInfo?.user_id || 1
        }
        const res: any = await request.post('/health/measurements', payload)
        if (res.code === 200) {
          ElMessage.success('录入成功')
          dialogFormVisible.value = false
          getMeasurements()
        }
      } finally {
        submitLoading.value = false
      }
    }
  })
}
</script>

<style scoped>
.text-danger { color: #F56C6C; font-weight: bold; }
.text-success { color: #67C23A; }
.card-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
  font-weight: bold;
}
</style>
