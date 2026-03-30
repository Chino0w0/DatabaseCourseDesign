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
    <el-card v-if="selectedResidentId" class="box-card" style="margin-top: 20px; margin-bottom: 20px;">
      <template #header>
        <div class="card-header">
          <span>健康档案摘要</span>
          <div style="display:flex; gap:8px; align-items:center;">
            <el-tag type="info">血型: {{ healthProfile?.blood_type || '未登记' }}</el-tag>
            <el-button type="primary" link icon="Edit" @click="handleEditProfile">编辑摘要</el-button>
          </div>
        </div>
      </template>
      <el-descriptions :column="3" border>
        <el-descriptions-item label="过敏史">{{ healthProfile?.allergy_history || '无' }}</el-descriptions-item>
        <el-descriptions-item label="家族病史">{{ healthProfile?.family_history || '无' }}</el-descriptions-item>
        <el-descriptions-item label="既往病史">{{ healthProfile?.past_medical_history || '无' }}</el-descriptions-item>
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
      <el-table-column prop="notes" label="备注" min-width="180" show-overflow-tooltip>
        <template #default="{ row }">
          {{ row.notes || '-' }}
        </template>
      </el-table-column>
      <el-table-column label="异常预警" width="200" show-overflow-tooltip>
        <template #default="{ row }">
          <el-tag v-if="row.warning" type="danger" size="small">警告</el-tag>
          <span v-if="row.warning" style="color:red; margin-left:5px; font-size:12px;">{{ formatWarningMsg(row.warning_msg) }}</span>
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
        <el-form-item label="身高(cm)" prop="height">
          <el-input-number v-model="temp.height" :min="30" :max="260" :precision="1" :step="0.1" />
        </el-form-item>
        <el-form-item label="体重(kg)" prop="weight">
          <el-input-number v-model="temp.weight" :min="1" :max="500" :precision="1" :step="0.1" />
        </el-form-item>
        <el-form-item label="测量时间" prop="measured_at">
          <el-date-picker
            v-model="temp.measured_at"
            type="datetime"
            value-format="YYYY-MM-DD HH:mm:ss"
            placeholder="请选择测量时间"
            style="width: 100%;"
          />
        </el-form-item>
        <el-form-item label="备注" prop="notes">
          <el-input v-model="temp.notes" type="textarea" :rows="2" placeholder="可选" />
        </el-form-item>
      </el-form>
      <template #footer>
        <div class="dialog-footer">
          <el-button @click="dialogFormVisible = false">取消</el-button>
          <el-button type="primary" :loading="submitLoading" @click="createData">确 定</el-button>
        </div>
      </template>
    </el-dialog>

    <el-dialog title="编辑健康档案摘要" v-model="profileDialogVisible" width="500px">
      <el-form :model="profileTemp" label-position="right" label-width="110px">
        <el-form-item label="血型">
          <el-select v-model="profileTemp.blood_type" placeholder="请选择或留空" clearable style="width: 100%;">
            <el-option label="A" value="A" />
            <el-option label="B" value="B" />
            <el-option label="AB" value="AB" />
            <el-option label="O" value="O" />
          </el-select>
        </el-form-item>
        <el-form-item label="过敏史">
          <el-input v-model="profileTemp.allergy_history" type="textarea" :rows="2" placeholder="无可留空" />
        </el-form-item>
        <el-form-item label="家族病史">
          <el-input v-model="profileTemp.family_history" type="textarea" :rows="2" placeholder="无可留空" />
        </el-form-item>
        <el-form-item label="既往病史">
          <el-input v-model="profileTemp.past_medical_history" type="textarea" :rows="2" placeholder="无可留空" />
        </el-form-item>
      </el-form>
      <template #footer>
        <div class="dialog-footer">
          <el-button @click="profileDialogVisible = false">取消</el-button>
          <el-button type="primary" :loading="profileSubmitLoading" @click="saveProfile">保存</el-button>
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
  heart_rate: 75,
  height: 170,
  weight: 65,
  measured_at: '',
  notes: ''
})

const profileDialogVisible = ref(false)
const profileSubmitLoading = ref(false)
const profileTemp = reactive({
  blood_type: '',
  allergy_history: '',
  family_history: '',
  past_medical_history: ''
})

const rules = reactive<FormRules>({
  systolic: [{ required: true, message: '必填', trigger: 'blur' }],
  diastolic: [{ required: true, message: '必填', trigger: 'blur' }],
  measured_at: [{ required: true, message: '请选择测量时间', trigger: 'change' }]
})

const handleCreate = () => {
  temp.systolic = 120
  temp.diastolic = 80
  temp.blood_sugar = 5.5
  temp.heart_rate = 75
  temp.height = 170
  temp.weight = 65
  temp.notes = ''
  temp.measured_at = new Date().toISOString().slice(0, 19).replace('T', ' ')
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
const handleEditProfile = () => {
  if (!selectedResidentId.value) {
    ElMessage.warning('请先选择居民')
    return
  }

  profileTemp.blood_type = healthProfile.value?.blood_type || ''
  profileTemp.allergy_history = healthProfile.value?.allergy_history || ''
  profileTemp.family_history = healthProfile.value?.family_history || ''
  profileTemp.past_medical_history = healthProfile.value?.past_medical_history || ''
  profileDialogVisible.value = true
}

const saveProfile = async () => {
  if (!selectedResidentId.value) {
    ElMessage.warning('请先选择居民')
    return
  }

  profileSubmitLoading.value = true
  try {
    const res: any = await request.put(`/health/records/${selectedResidentId.value}`, profileTemp)
    if (res.code === 200) {
      ElMessage.success('健康档案摘要已更新')
      profileDialogVisible.value = false
      await getHealthProfile()
    }
  } finally {
    profileSubmitLoading.value = false
  }
}

const formatWarningMsg = (msg: any) => {
  if (Array.isArray(msg)) return msg.join('; ')
  if (typeof msg === 'string') return msg
  return ''
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
