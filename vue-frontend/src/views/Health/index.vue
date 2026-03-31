<template>
  <div class="app-container">
    <div class="filter-container" style="display:flex; gap:15px; align-items:center; flex-wrap: wrap;">
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

      <el-button
        v-if="canEditHealthProfile"
        type="warning"
        icon="Edit"
        :disabled="!selectedResidentId || !healthProfile"
        @click="handleEditHealthProfile"
      >
        编辑健康档案
      </el-button>
    </div>

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
          <span :class="{ 'text-danger': row.systolic > 140 || row.diastolic > 90 }">
            {{ row.systolic }} / {{ row.diastolic }}
          </span>
        </template>
      </el-table-column>
      <el-table-column prop="blood_sugar" label="血糖 (mmol/L)" align="center" />
      <el-table-column prop="heart_rate" label="心率 (次/分)" align="center" />
      <el-table-column label="身高/体重" align="center">
        <template #default="{ row }">
          {{ row.height || '--' }}cm / {{ row.weight || '--' }}kg
        </template>
      </el-table-column>
      <el-table-column prop="bmi" label="BMI" align="center">
        <template #default="{ row }">
          <el-tag :type="getBmiType(row.bmi)">{{ row.bmi || '--' }}</el-tag>
        </template>
      </el-table-column>
      <el-table-column prop="measured_by" label="测量人" align="center" />
      <el-table-column label="异常预警" width="260" show-overflow-tooltip>
        <template #default="{ row }">
          <el-tag v-if="row.warning" type="danger" size="small">警告</el-tag>
          <span v-if="row.warning" style="color:red; margin-left:5px; font-size:12px;">{{ row.warning_msg?.join('; ') }}</span>
          <span v-else class="text-success" style="font-size:12px;">正常</span>
        </template>
      </el-table-column>
      <el-table-column v-if="userStore.canWriteMeasurement" label="操作" width="120" align="center" fixed="right">
        <template #default="{ row }">
          <el-button type="primary" size="small" icon="Edit" @click="handleEdit(row)">编辑</el-button>
        </template>
      </el-table-column>
    </el-table>

    <el-dialog title="编辑健康档案摘要" v-model="profileDialogVisible" width="620px">
      <el-form ref="profileFormRef" :rules="profileRules" :model="profileTemp" label-position="right" label-width="110px">
        <el-row :gutter="20">
          <el-col :span="12">
            <el-form-item label="血型" prop="blood_type">
              <el-select v-model="profileTemp.blood_type" placeholder="请选择血型" style="width: 100%;" clearable>
                <el-option label="A型" value="A型" />
                <el-option label="B型" value="B型" />
                <el-option label="O型" value="O型" />
                <el-option label="AB型" value="AB型" />
              </el-select>
            </el-form-item>
          </el-col>
        </el-row>
        <el-form-item label="过敏史" prop="allergy_history">
          <el-input v-model="profileTemp.allergy_history" type="textarea" :rows="3" placeholder="请输入过敏史" />
        </el-form-item>
        <el-form-item label="家族病史" prop="family_history">
          <el-input v-model="profileTemp.family_history" type="textarea" :rows="3" placeholder="请输入家族病史" />
        </el-form-item>
        <el-form-item label="既往病史" prop="past_medical_history">
          <el-input v-model="profileTemp.past_medical_history" type="textarea" :rows="3" placeholder="请输入既往病史" />
        </el-form-item>
      </el-form>
      <template #footer>
        <div class="dialog-footer">
          <el-button @click="profileDialogVisible = false">取消</el-button>
          <el-button type="primary" :loading="profileSubmitLoading" @click="updateHealthProfile">确 定</el-button>
        </div>
      </template>
    </el-dialog>

    <el-dialog :title="dialogStatus === 'create' ? '新增体检数据' : '编辑体检数据'" v-model="dialogFormVisible" width="560px">
      <el-form ref="dataFormRef" :rules="rules" :model="temp" label-position="right" label-width="120px">
        <el-form-item label="测量时间" prop="measured_at">
          <el-date-picker
            v-model="temp.measured_at"
            type="datetime"
            placeholder="选择测量时间"
            value-format="YYYY-MM-DD HH:mm:ss"
            style="width: 100%;"
          />
        </el-form-item>
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
        <el-form-item label="备注" prop="notes">
          <el-input v-model="temp.notes" type="textarea" :rows="3" placeholder="请输入备注信息" />
        </el-form-item>
      </el-form>
      <template #footer>
        <div class="dialog-footer">
          <el-button @click="dialogFormVisible = false">取消</el-button>
          <el-button type="primary" :loading="submitLoading" @click="submitMeasurement">确 定</el-button>
        </div>
      </template>
    </el-dialog>
  </div>
</template>

<script setup lang="ts">
import { computed, ref, reactive, onMounted, watch } from 'vue'
import { useRoute, useRouter } from 'vue-router'
import request from '@/utils/request'
import { ElMessage } from 'element-plus'
import type { FormInstance, FormRules } from 'element-plus'
import { useUserStore } from '@/store/user'

const userStore = useUserStore()
const route = useRoute()
const router = useRouter()
const healthSearchStorageKey = 'health_view_query'

const canEditHealthProfile = computed(() => userStore.isAdmin || userStore.isDoctor)

const storedHealthQuery = (() => {
  try {
    return JSON.parse(sessionStorage.getItem(healthSearchStorageKey) || 'null')
  } catch (_err) {
    return null
  }
})()

const searchLoading = ref(false)
const residentOptions = ref<any[]>([])
const selectedResidentId = ref<number | undefined>(
  Number(route.query.resident_id || storedHealthQuery?.resident_id || 0) > 0
    ? Number(route.query.resident_id || storedHealthQuery?.resident_id)
    : undefined
)

const listLoading = ref(false)
const measurements = ref<any[]>([])
const healthProfile = ref<any>(null)

const ensureResidentOption = async (residentId: number) => {
  if (residentOptions.value.some(item => item.id === residentId)) {
    return
  }
  try {
    const res: any = await request.get(`/residents/${residentId}`)
    if (res.code === 200 && res.data) {
      residentOptions.value = [res.data, ...residentOptions.value.filter(item => item.id !== residentId)]
    }
  } catch (_err) {}
}

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

const persistResidentSelection = () => {
  if (selectedResidentId.value) {
    sessionStorage.setItem(healthSearchStorageKey, JSON.stringify({ resident_id: selectedResidentId.value }))
  } else {
    sessionStorage.removeItem(healthSearchStorageKey)
  }
}

const syncResidentQuery = () => {
  const residentId = selectedResidentId.value
  const currentResidentId = route.query.resident_id

  if (!residentId) {
    const nextQuery = { ...route.query }
    delete nextQuery.resident_id
    router.replace({ query: nextQuery })
    return
  }

  if (String(currentResidentId || '') !== String(residentId)) {
    router.replace({ query: { ...route.query, resident_id: String(residentId) } })
  }
}

const handleResidentSelect = () => {
  syncResidentQuery()
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

const profileDialogVisible = ref(false)
const profileFormRef = ref<FormInstance>()
const profileSubmitLoading = ref(false)
const profileTemp = reactive({
  blood_type: '',
  allergy_history: '',
  family_history: '',
  past_medical_history: ''
})
const profileRules = reactive<FormRules>({})

const handleEditHealthProfile = () => {
  if (!canEditHealthProfile.value) {
    ElMessage.warning('当前角色无权编辑健康档案')
    return
  }
  if (!selectedResidentId.value || !healthProfile.value) {
    ElMessage.warning('请先选择居民并加载健康档案')
    return
  }
  profileTemp.blood_type = healthProfile.value.blood_type || ''
  profileTemp.allergy_history = healthProfile.value.allergy_history || ''
  profileTemp.family_history = healthProfile.value.family_history || ''
  profileTemp.past_medical_history = healthProfile.value.past_medical_history || ''
  profileDialogVisible.value = true
  setTimeout(() => profileFormRef.value?.clearValidate(), 0)
}

const updateHealthProfile = () => {
  if (!canEditHealthProfile.value) {
    ElMessage.warning('当前角色无权编辑健康档案')
    return
  }
  if (!selectedResidentId.value) {
    ElMessage.warning('请先选择居民')
    return
  }

  profileFormRef.value?.validate(async (valid) => {
    if (valid) {
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
  })
}

const dialogFormVisible = ref(false)
const dialogStatus = ref<'create' | 'update'>('create')
const dataFormRef = ref<FormInstance>()
const submitLoading = ref(false)

const getNowDateTime = () => {
  const d = new Date()
  return `${d.getFullYear()}-${String(d.getMonth() + 1).padStart(2, '0')}-${String(d.getDate()).padStart(2, '0')} ${String(d.getHours()).padStart(2, '0')}:${String(d.getMinutes()).padStart(2, '0')}:${String(d.getSeconds()).padStart(2, '0')}`
}

const temp = reactive({
  id: undefined as number | undefined,
  measured_at: getNowDateTime(),
  systolic: 120,
  diastolic: 80,
  blood_sugar: 5.5,
  heart_rate: 75,
  height: 170,
  weight: 60,
  notes: ''
})

const rules = reactive<FormRules>({
  measured_at: [{ required: true, message: '请选择测量时间', trigger: 'change' }],
  systolic: [{ required: true, message: '必填', trigger: 'blur' }],
  diastolic: [{ required: true, message: '必填', trigger: 'blur' }]
})

const handleCreate = () => {
  if (!userStore.canWriteMeasurement) {
    ElMessage.warning('当前角色无权录入体检数据')
    return
  }
  if (!selectedResidentId.value) {
    ElMessage.warning('请先选择居民再录入体检数据')
    return
  }
  dialogStatus.value = 'create'
  temp.id = undefined
  temp.measured_at = getNowDateTime()
  temp.systolic = 120
  temp.diastolic = 80
  temp.blood_sugar = 5.5
  temp.heart_rate = 75
  temp.height = 170
  temp.weight = 60
  temp.notes = ''
  dialogFormVisible.value = true
  setTimeout(() => dataFormRef.value?.clearValidate(), 0)
}

const handleEdit = (row: any) => {
  if (!userStore.canWriteMeasurement) {
    ElMessage.warning('当前角色无权编辑体检数据')
    return
  }
  if (!selectedResidentId.value) {
    ElMessage.warning('请先选择居民')
    return
  }
  dialogStatus.value = 'update'
  temp.id = row.id
  temp.measured_at = row.measured_at || getNowDateTime()
  temp.systolic = row.systolic ?? 120
  temp.diastolic = row.diastolic ?? 80
  temp.blood_sugar = row.blood_sugar ?? 5.5
  temp.heart_rate = row.heart_rate ?? 75
  temp.height = row.height ?? 170
  temp.weight = row.weight ?? 60
  temp.notes = row.notes || ''
  dialogFormVisible.value = true
  setTimeout(() => dataFormRef.value?.clearValidate(), 0)
}

const submitMeasurement = () => {
  if (!userStore.canWriteMeasurement) {
    ElMessage.warning(dialogStatus.value === 'create' ? '当前角色无权录入体检数据' : '当前角色无权编辑体检数据')
    return
  }
  if (!selectedResidentId.value) {
    ElMessage.warning('请先选择居民再保存体检数据')
    return
  }

  dataFormRef.value?.validate(async (valid) => {
    if (valid) {
      submitLoading.value = true
      try {
        const payload = {
          measured_at: temp.measured_at,
          systolic: temp.systolic,
          diastolic: temp.diastolic,
          blood_sugar: temp.blood_sugar,
          heart_rate: temp.heart_rate,
          height: temp.height,
          weight: temp.weight,
          notes: temp.notes,
          resident_id: selectedResidentId.value,
          measured_by_user_id: userStore.userInfo?.id || userStore.userInfo?.user_id || 1
        }
        const req = dialogStatus.value === 'create'
          ? request.post('/health/measurements', payload)
          : request.put(`/health/measurements/${temp.id}`, payload)
        const res: any = await req
        if (res.code === 200) {
          ElMessage.success(dialogStatus.value === 'create' ? '录入成功' : '更新成功')
          dialogFormVisible.value = false
          await getMeasurements()
        }
      } finally {
        submitLoading.value = false
      }
    }
  })
}

onMounted(async () => {
  if (selectedResidentId.value) {
    await ensureResidentOption(selectedResidentId.value)
    handleResidentSelect()
  }
})

watch(selectedResidentId, () => {
  syncResidentQuery()
  persistResidentSelection()
})
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
