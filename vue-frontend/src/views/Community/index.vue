<template>
  <div class="app-container">
    <div class="filter-container">
      <el-button
        v-if="userStore.canManageCommunity"
        class="filter-item"
        type="success"
        icon="Plus"
        @click="handleCreate"
      >
        新增社区
      </el-button>
    </div>

    <el-table v-loading="listLoading" :data="list" border stripe style="width: 100%">
      <el-table-column prop="id" label="ID" width="80" align="center" />
      <el-table-column prop="name" label="社区名称" min-width="180" />
      <el-table-column prop="address" label="社区地址" min-width="260" show-overflow-tooltip />
      <el-table-column prop="contact_phone" label="联系电话" width="160" />
      <el-table-column prop="created_at" label="创建时间" width="180" align="center" />
      <el-table-column v-if="userStore.canManageCommunity" label="操作" width="120" align="center" fixed="right">
        <template #default="{ row }">
          <el-button type="primary" size="small" icon="Edit" @click="handleUpdate(row)">编辑</el-button>
        </template>
      </el-table-column>
    </el-table>

    <el-dialog :title="dialogStatus === 'create' ? '新增社区' : '编辑社区'" v-model="dialogFormVisible" width="520px">
      <el-form ref="dataFormRef" :rules="rules" :model="temp" label-position="right" label-width="100px">
        <el-form-item label="社区名称" prop="name">
          <el-input v-model="temp.name" placeholder="请输入社区名称" />
        </el-form-item>
        <el-form-item label="社区地址" prop="address">
          <el-input v-model="temp.address" placeholder="请输入社区地址" />
        </el-form-item>
        <el-form-item label="联系电话" prop="contact_phone">
          <el-input v-model="temp.contact_phone" placeholder="请输入联系电话" />
        </el-form-item>
      </el-form>
      <template #footer>
        <span class="dialog-footer">
          <el-button @click="dialogFormVisible = false">取消</el-button>
          <el-button type="primary" :loading="submitLoading" @click="submitData">确 定</el-button>
        </span>
      </template>
    </el-dialog>
  </div>
</template>

<script setup lang="ts">
import { onMounted, reactive, ref } from 'vue'
import { ElMessage } from 'element-plus'
import type { FormInstance, FormRules } from 'element-plus'
import request from '@/utils/request'
import { useUserStore } from '@/store/user'

const userStore = useUserStore()

const listLoading = ref(false)
const list = ref<any[]>([])

const dialogFormVisible = ref(false)
const dialogStatus = ref<'create' | 'update'>('create')
const dataFormRef = ref<FormInstance>()
const submitLoading = ref(false)

const temp = reactive({
  id: undefined as number | undefined,
  name: '',
  address: '',
  contact_phone: ''
})

const phonePattern = /^(1[3-9]\d{9}|0\d{2,3}-?\d{7,8})$/

const rules = reactive<FormRules>({
  name: [{ required: true, message: '请输入社区名称', trigger: 'blur' }],
  contact_phone: [
    {
      validator: (_rule, value, callback) => {
        if (!value) {
          callback()
          return
        }
        if (!phonePattern.test(String(value))) {
          callback(new Error('联系电话格式不正确，应为11位手机号或区号-座机号'))
          return
        }
        callback()
      },
      trigger: 'blur'
    }
  ]
})

const resetTemp = () => {
  temp.id = undefined
  temp.name = ''
  temp.address = ''
  temp.contact_phone = ''
}

const getList = async () => {
  listLoading.value = true
  try {
    const res: any = await request.get('/communities')
    if (res.code === 200) {
      list.value = Array.isArray(res.data) ? res.data : (res.data.list || [])
    }
  } finally {
    listLoading.value = false
  }
}

const handleCreate = () => {
  if (!userStore.canManageCommunity) {
    ElMessage.warning('当前角色无权新增社区')
    return
  }
  resetTemp()
  dialogStatus.value = 'create'
  dialogFormVisible.value = true
  setTimeout(() => dataFormRef.value?.clearValidate(), 0)
}

const handleUpdate = (row: any) => {
  if (!userStore.canManageCommunity) {
    ElMessage.warning('当前角色无权编辑社区')
    return
  }
  temp.id = row.id
  temp.name = row.name || ''
  temp.address = row.address || ''
  temp.contact_phone = row.contact_phone || ''
  dialogStatus.value = 'update'
  dialogFormVisible.value = true
  setTimeout(() => dataFormRef.value?.clearValidate(), 0)
}

const submitData = () => {
  dataFormRef.value?.validate(async (valid) => {
    if (!valid) return
    submitLoading.value = true
    try {
      if (dialogStatus.value === 'create') {
        const res: any = await request.post('/communities', {
          name: temp.name,
          address: temp.address,
          contact_phone: temp.contact_phone
        })
        if (res.code === 200) {
          ElMessage.success('社区新增成功')
          dialogFormVisible.value = false
          getList()
        }
      } else {
        const res: any = await request.put(`/communities/${temp.id}`, {
          name: temp.name,
          address: temp.address,
          contact_phone: temp.contact_phone
        })
        if (res.code === 200) {
          ElMessage.success('社区更新成功')
          dialogFormVisible.value = false
          getList()
        }
      }
    } finally {
      submitLoading.value = false
    }
  })
}

onMounted(() => {
  getList()
})
</script>
