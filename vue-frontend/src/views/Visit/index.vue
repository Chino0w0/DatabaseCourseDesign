<template>
  <div class="app-container">
    <div class="filter-container" style="display:flex; gap:15px; align-items:center;">
      <el-select
        v-model="selectedResidentId"
        filterable
        remote
        reserve-keyword
        placeholder="请输入居民姓名搜索随访记录"
        :remote-method="searchResidents"
        :loading="searchLoading"
        style="width: 300px"
        @change="getVisits"
      >
        <el-option
          v-for="item in residentOptions"
          :key="item.id"
          :label="`${item.name} (${item.id_card.slice(-4)})`"
          :value="item.id"
        />
      </el-select>

      <el-button type="primary" icon="Search" :disabled="!selectedResidentId" @click="getVisits">
        查询随访记录
      </el-button>
      
      <el-button
        v-if="userStore.isNurse || userStore.isAdmin"
        type="success"
        icon="EditPen"
        :disabled="!selectedResidentId"
        @click="handleCreate"
      >
        新增随访记录
      </el-button>
    </div>

    <el-table
      v-loading="listLoading"
      :data="visits"
      border
      stripe
      style="width: 100%; margin-top: 20px;"
      height="500"
    >
      <el-table-column prop="id" label="ID" width="60" align="center" />
      <el-table-column prop="visit_date" label="随访时间" width="120" align="center" />
      <el-table-column prop="visit_type" label="随访方式" width="120" align="center">
        <template #default="{ row }">
          <el-tag :type="row.visit_type === '上门随访' ? 'primary' : 'success'">{{ row.visit_type }}</el-tag>
        </template>
      </el-table-column>
      <el-table-column prop="content" label="随访内容" min-width="250" show-overflow-tooltip />
      <el-table-column prop="visitor_name" label="随访人" width="120" align="center" />
      <el-table-column prop="next_visit_date" label="下次随访日期" width="150" align="center">
         <template #default="{ row }">
          <span style="color: #E6A23C; font-weight: bold;">{{ row.next_visit_date || '未安排' }}</span>
        </template>
      </el-table-column>
    </el-table>

    <!-- 录入随访数据弹窗 -->
    <el-dialog title="新增随访记录" v-model="dialogFormVisible" width="600px">
      <el-form ref="dataFormRef" :rules="rules" :model="temp" label-position="top">
        <el-row :gutter="20">
          <el-col :span="12">
            <el-form-item label="随访日期" prop="visit_date">
              <el-date-picker 
                v-model="temp.visit_date" 
                type="date" 
                placeholder="选择日期" 
                value-format="YYYY-MM-DD"
                style="width: 100%;"
              />
            </el-form-item>
          </el-col>
          <el-col :span="12">
            <el-form-item label="随访方式" prop="visit_type">
              <el-select v-model="temp.visit_type" placeholder="请选择随访方式" style="width: 100%;">
                <el-option label="上门随访" value="上门随访" />
                <el-option label="电话随访" value="电话随访" />
                <el-option label="门诊随访" value="门诊随访" />
              </el-select>
            </el-form-item>
          </el-col>
        </el-row>
        
        <el-form-item label="随访内容" prop="content">
          <el-input 
            v-model="temp.content" 
            type="textarea" 
            :rows="4" 
            placeholder="详细记录患者此次随访的身体状况、用药情况等..." 
          />
        </el-form-item>

        <el-row :gutter="20">
          <el-col :span="12">
            <el-form-item label="下次预约随访日期" prop="next_visit_date">
              <el-date-picker 
                v-model="temp.next_visit_date" 
                type="date" 
                placeholder="选择日期" 
                value-format="YYYY-MM-DD"
                style="width: 100%;"
              />
            </el-form-item>
          </el-col>
          <el-col :span="12">
            <el-form-item label="随访医生/护士" prop="visitor_name">
              <el-input :value="userStore.userInfo?.real_name || userStore.userInfo?.username" disabled />
            </el-form-item>
          </el-col>
        </el-row>
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
const visits = ref<any[]>([])

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

const getVisits = async () => {
  if (!selectedResidentId.value) return
  listLoading.value = true
  try {
    const res: any = await request.get('/visits', { params: { resident_id: selectedResidentId.value } })
    if (res.code === 200) {
      visits.value = res.data || []
    }
  } finally {
    listLoading.value = false
  }
}

// 录入数据弹窗逻辑
const dialogFormVisible = ref(false)
const dataFormRef = ref<FormInstance>()
const submitLoading = ref(false)

const getTodayDate = () => {
  const d = new Date()
  return `${d.getFullYear()}-${String(d.getMonth()+1).padStart(2,'0')}-${String(d.getDate()).padStart(2,'0')}`
}

const temp = reactive({
  visit_date: getTodayDate(),
  visit_type: '上门随访',
  content: '',
  next_visit_date: ''
})

const rules = reactive<FormRules>({
  visit_date: [{ required: true, message: '请选择随访日期', trigger: 'change' }],
  visit_type: [{ required: true, message: '请选择随访方式', trigger: 'change' }],
  content: [{ required: true, message: '请输入随访内容', trigger: 'blur' }]
})

const handleCreate = () => {
  if (!(userStore.isNurse || userStore.isAdmin)) {
    ElMessage.warning('仅管理员和护士可新增随访档案')
    return
  }

  temp.visit_date = getTodayDate()
  temp.visit_type = '上门随访'
  temp.content = ''
  temp.next_visit_date = ''
  dialogFormVisible.value = true
  setTimeout(() => dataFormRef.value?.clearValidate(), 0)
}

const createData = () => {
  if (!(userStore.isNurse || userStore.isAdmin)) {
    ElMessage.warning('仅管理员和护士可新增随访档案')
    return
  }

  dataFormRef.value?.validate(async (valid) => {
    if (valid) {
      submitLoading.value = true
      try {
        const payload = {
          ...temp,
          resident_id: selectedResidentId.value,
          visitor_user_id: userStore.userInfo?.id || userStore.userInfo?.user_id || 1
        }
        const res: any = await request.post('/visits', payload)
        if (res.code === 200) {
          ElMessage.success('随访记录已保存')
          dialogFormVisible.value = false
          getVisits()
        }
      } finally {
        submitLoading.value = false
      }
    }
  })
}
</script>
