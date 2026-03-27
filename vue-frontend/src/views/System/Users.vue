<template>
  <div class="app-container">
    <div class="filter-container">
      <el-button class="filter-item" type="primary" icon="Plus" @click="handleCreate">
        新增系统账户
      </el-button>
    </div>

    <el-table
      v-loading="listLoading"
      :data="list"
      border
      stripe
      style="width: 100%"
    >
      <el-table-column prop="id" label="ID" width="80" align="center" />
      <el-table-column prop="username" label="登录账号" width="150" />
      <el-table-column prop="real_name" label="真实姓名" width="150" />
      <el-table-column prop="role_name" label="系统角色" width="150" align="center">
        <template #default="{ row }">
          <!-- 1=Admin, 2=Doctor, 3=Nurse (based on standard convention in earlier scripts) -->
          <el-tag :type="row.role_id === 1 ? 'danger' : (row.role_id === 2 ? 'success' : 'info')">
            {{ row.role_name || (row.role_id === 1 ? '管理员' : (row.role_id === 2 ? '医生' : '护士')) }}
          </el-tag>
        </template>
      </el-table-column>
      <el-table-column prop="phone" label="联系电话" width="150" />
      <el-table-column prop="status" label="状态" width="100" align="center">
        <template #default="{ row }">
          <el-switch
            v-model="row.status"
            :active-value="1"
            :inactive-value="0"
            @change="handleStatusChange(row)"
            :disabled="row.id === currentUserId"
          />
        </template>
      </el-table-column>
      <el-table-column prop="created_at" label="创建时间" min-width="180" />
      <el-table-column label="操作" width="200" align="center" fixed="right">
        <template #default="{ row }">
          <el-button type="primary" size="small" icon="Edit" @click="handleUpdate(row)">编辑</el-button>
          <el-button 
            type="danger" 
            size="small" 
            icon="Delete" 
            @click="handleDelete(row)"
            :disabled="row.id === currentUserId"
          >删除</el-button>
        </template>
      </el-table-column>
    </el-table>

    <div style="margin-top: 20px; display: flex; justify-content: flex-end;">
      <el-pagination
        v-model:current-page="listQuery.page"
        v-model:page-size="listQuery.size"
        :page-sizes="[10, 20, 50, 100]"
        background
        layout="total, sizes, prev, pager, next, jumper"
        :total="total"
        @size-change="handleSizeChange"
        @current-change="handleCurrentChange"
      />
    </div>

    <el-dialog :title="dialogStatus==='create' ? '新增操作员账户' : '编辑操作员账户'" v-model="dialogFormVisible" width="500px">
      <el-form ref="dataFormRef" :rules="rules" :model="temp" label-position="right" label-width="100px">
        <el-form-item label="登录账号" prop="username">
          <el-input v-model="temp.username" placeholder="请输入唯一登录名" :disabled="dialogStatus==='update'" />
        </el-form-item>
        <el-form-item label="真实姓名" prop="real_name">
          <el-input v-model="temp.real_name" placeholder="对应的真实姓名 (例: 张医生)" />
        </el-form-item>
        <el-form-item label="登录密码" :prop="dialogStatus==='create' ? 'password' : ''">
          <el-input 
            v-model="temp.password" 
            type="password" 
            :placeholder="dialogStatus==='create' ? '请设置登录密码' : '留空则不修改密码'" 
            show-password 
          />
        </el-form-item>
        <el-form-item label="分配角色" prop="role_id">
          <el-select v-model="temp.role_id" placeholder="请选择角色" style="width: 100%;">
            <el-option label="系统管理员" :value="1" />
            <el-option label="主治医生" :value="2" />
            <el-option label="护士/社区人员" :value="3" />
          </el-select>
        </el-form-item>
        <el-form-item label="联系电话" prop="phone">
          <el-input v-model="temp.phone" placeholder="选填联系方式" />
        </el-form-item>
      </el-form>
      <template #footer>
        <span class="dialog-footer">
          <el-button @click="dialogFormVisible = false">取消</el-button>
          <el-button type="primary" :loading="submitLoading" @click="dialogStatus==='create' ? createData() : updateData()">确 定</el-button>
        </span>
      </template>
    </el-dialog>
  </div>
</template>

<script setup lang="ts">
import { ref, reactive, onMounted } from 'vue'
import { ElMessage, ElMessageBox } from 'element-plus'
import type { FormInstance, FormRules } from 'element-plus'
import request from '@/utils/request'
import { useUserStore } from '@/store/user'

const userStore = useUserStore()
const currentUserId = userStore.userInfo?.id || userStore.userInfo?.user_id

const listLoading = ref(false)
const list = ref([])
const total = ref(0)

const listQuery = reactive({
  page: 1,
  size: 10
})

const getList = async () => {
  listLoading.value = true
  try {
    const res: any = await request.get('/users', { params: listQuery })
    if (res.code === 200) {
      list.value = res.data.list || []
      total.value = res.data.total || 0
    }
  } finally {
    listLoading.value = false
  }
}

const handleSizeChange = (val: number) => {
  listQuery.size = val
  getList()
}

const handleCurrentChange = (val: number) => {
  listQuery.page = val
  getList()
}

const handleStatusChange = async (row: any) => {
  try {
    const res: any = await request.put(`/users/${row.id}`, { status: row.status })
    if (res.code === 200) {
      ElMessage.success('状态已更新')
    } else {
      row.status = row.status === 1 ? 0 : 1 // 还原
    }
  } catch (err) {
    row.status = row.status === 1 ? 0 : 1
  }
}

// 弹窗表单控制
const dialogFormVisible = ref(false)
const dialogStatus = ref('create')
const dataFormRef = ref<FormInstance>()
const submitLoading = ref(false)

const temp = reactive({
  id: undefined,
  username: '',
  password: '',
  real_name: '',
  phone: '',
  role_id: undefined as number | undefined
})

const rules = reactive<FormRules>({
  username: [{ required: true, message: '必填项', trigger: 'blur' }],
  real_name: [{ required: true, message: '必填项', trigger: 'blur' }],
  password: [{ required: true, message: '必填项', trigger: 'blur' }],
  role_id: [{ required: true, message: '请选择系统角色', trigger: 'change' }]
})

const resetTemp = () => {
  temp.id = undefined
  temp.username = ''
  temp.password = ''
  temp.real_name = ''
  temp.phone = ''
  temp.role_id = undefined
}

const handleCreate = () => {
  resetTemp()
  dialogStatus.value = 'create'
  dialogFormVisible.value = true
  setTimeout(() => dataFormRef.value?.clearValidate(), 0)
}

const createData = () => {
  dataFormRef.value?.validate(async (valid) => {
    if (valid) {
      submitLoading.value = true
      try {
        const res: any = await request.post('/users', temp)
        if (res.code === 200) {
          ElMessage.success('账号创建成功')
          dialogFormVisible.value = false
          getList()
        }
      } finally {
        submitLoading.value = false
      }
    }
  })
}

const handleUpdate = (row: any) => {
  Object.assign(temp, row) // 浅拷贝回显
  temp.password = '' // 编辑不回显密码
  
  if (!temp.role_id) temp.role_id = 2

  dialogStatus.value = 'update'
  dialogFormVisible.value = true
  setTimeout(() => dataFormRef.value?.clearValidate(), 0)
}

const updateData = () => {
  dataFormRef.value?.validate(async (valid) => {
    if (valid) {
      submitLoading.value = true
      try {
        const payload: any = {
          real_name: temp.real_name,
          role_id: temp.role_id,
          phone: temp.phone
        }
        if (temp.password) payload.password = temp.password

        const res: any = await request.put(`/users/${temp.id}`, payload)
        if (res.code === 200) {
          ElMessage.success('更新成功')
          dialogFormVisible.value = false
          getList()
        }
      } finally {
        submitLoading.value = false
      }
    }
  })
}

const handleDelete = (row: any) => {
  ElMessageBox.confirm(`确定要永远删除账户 ${row.username} 吗? 如果该用户还有关联的体检数据，建议选择【禁用状态】而不是直接删除。`, '高危操作', {
    confirmButtonText: '确定吊销并删除',
    cancelButtonText: '取消',
    type: 'error'
  }).then(async () => {
    const res: any = await request.delete(`/users/${row.id}`)
    if (res.code === 200) {
      ElMessage.success('账户删除成功')
      if (list.value.length === 1 && listQuery.page > 1) {
        listQuery.page--
      }
      getList()
    }
  }).catch(() => {})
}

onMounted(() => {
  getList()
})
</script>
