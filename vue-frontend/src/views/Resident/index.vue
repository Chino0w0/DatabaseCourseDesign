<template>
  <div class="app-container">
    <div class="filter-container">
      <el-input 
        v-model="listQuery.keyword" 
        placeholder="搜索居民姓名/身份证号" 
        style="width: 300px;" 
        class="filter-item"
        clearable
        @keyup.enter="handleFilter"
      />
      <el-button class="filter-item" type="primary" icon="Search" @click="handleFilter">
        搜索
      </el-button>
      <el-button class="filter-item" type="success" icon="Plus" @click="handleCreate">
        新增居民档案
      </el-button>
    </div>

    <el-table
      v-loading="listLoading"
      :data="list"
      border
      stripe
      style="width: 100%"
    >
      <el-table-column prop="id" label="ID" width="60" align="center" />
      <el-table-column prop="name" label="姓名" width="120" />
      <el-table-column prop="gender" label="性别" width="80" align="center" />
      <el-table-column prop="birth_date" label="出生日期" width="120" align="center" />
      <el-table-column prop="phone" label="联系电话" width="130" />
      <el-table-column prop="community_name" label="所属社区" width="150" />
      <el-table-column prop="address" label="详细地址" min-width="200" show-overflow-tooltip />
      <el-table-column label="操作" width="220" align="center" fixed="right">
        <template #default="{ row }">
          <el-button type="primary" size="small" icon="Edit" @click="handleUpdate(row)">编辑</el-button>
          <el-button type="danger" size="small" icon="Delete" @click="handleDelete(row)">删除</el-button>
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

    <!-- 新增/编辑弹窗 -->
    <el-dialog :title="dialogStatus==='create' ? '新增居民档案' : '编辑居民档案'" v-model="dialogFormVisible" width="600px">
      <el-form ref="dataFormRef" :rules="rules" :model="temp" label-position="right" label-width="100px">
        <el-row :gutter="20">
          <el-col :span="12">
            <el-form-item label="姓名" prop="name">
              <el-input v-model="temp.name" placeholder="请输入姓名"/>
            </el-form-item>
          </el-col>
          <el-col :span="12">
            <el-form-item label="性别" prop="gender">
              <el-select v-model="temp.gender" placeholder="请选择性别" style="width: 100%;">
                <el-option label="男" value="男" />
                <el-option label="女" value="女" />
              </el-select>
            </el-form-item>
          </el-col>
        </el-row>
        
        <el-row :gutter="20">
          <el-col :span="12">
            <el-form-item label="身份证号" prop="id_card">
              <el-input v-model="temp.id_card" placeholder="请输入18位身份证号"/>
            </el-form-item>
          </el-col>
          <el-col :span="12">
            <el-form-item label="出生日期" prop="birth_date">
              <el-date-picker 
                v-model="temp.birth_date" 
                type="date" 
                placeholder="选择出生日期" 
                value-format="YYYY-MM-DD"
                style="width: 100%;"
              />
            </el-form-item>
          </el-col>
        </el-row>

        <el-row :gutter="20">
          <el-col :span="12">
            <el-form-item label="联系电话" prop="phone">
              <el-input v-model="temp.phone" placeholder="请输入手机号"/>
            </el-form-item>
          </el-col>
          <el-col :span="12">
            <el-form-item label="所属社区" prop="community_id">
              <el-select v-model="temp.community_id" placeholder="请选择社区" style="width: 100%;">
                <el-option v-for="item in communities" :key="item.id" :label="item.name" :value="item.id" />
              </el-select>
            </el-form-item>
          </el-col>
        </el-row>

        <el-form-item label="详细地址" prop="address">
          <el-input v-model="temp.address" placeholder="请输入详细居住地址" />
        </el-form-item>

        <el-row :gutter="20">
          <el-col :span="12">
            <el-form-item label="紧急联系人" prop="emergency_contact">
              <el-input v-model="temp.emergency_contact" placeholder="姓名"/>
            </el-form-item>
          </el-col>
          <el-col :span="12">
            <el-form-item label="紧急电话" prop="emergency_phone">
              <el-input v-model="temp.emergency_phone" placeholder="手机号"/>
            </el-form-item>
          </el-col>
        </el-row>
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

const listLoading = ref(false)
const list = ref([])
const total = ref(0)
const communities = ref<any[]>([])

const listQuery = reactive({
  page: 1,
  size: 10,
  keyword: undefined
})

const getList = async () => {
  listLoading.value = true
  try {
    const res: any = await request.get('/residents', { params: listQuery })
    if (res.code === 200) {
      list.value = res.data.list || []
      total.value = res.data.total || 0
    }
  } finally {
    listLoading.value = false
  }
}

const getCommunities = async () => {
  try {
    const res: any = await request.get('/communities')
    if (res.code === 200) communities.value = res.data.list || []
  } catch (err) {}
}

const handleFilter = () => {
  listQuery.page = 1
  getList()
}

const handleSizeChange = (val: number) => {
  listQuery.size = val
  getList()
}

const handleCurrentChange = (val: number) => {
  listQuery.page = val
  getList()
}

// 弹窗表单控制
const dialogFormVisible = ref(false)
const dialogStatus = ref('create')
const dataFormRef = ref<FormInstance>()
const submitLoading = ref(false)

const temp = reactive({
  id: undefined,
  name: '',
  gender: '',
  id_card: '',
  phone: '',
  birth_date: '',
  community_id: undefined as number | undefined,
  address: '',
  emergency_contact: '',
  emergency_phone: ''
})

const rules = reactive<FormRules>({
  name: [{ required: true, message: '必填项', trigger: 'blur' }],
  id_card: [{ required: true, message: '必填项', trigger: 'blur' }, { min: 18, max: 18, message: '请输入18位身份证号', trigger: 'blur' }],
  phone: [{ required: true, message: '必填项', trigger: 'blur' }],
  community_id: [{ required: true, message: '请选择所属社区', trigger: 'change' }]
})

const resetTemp = () => {
  temp.id = undefined
  temp.name = ''
  temp.gender = ''
  temp.id_card = ''
  temp.phone = ''
  temp.birth_date = ''
  temp.community_id = undefined
  temp.address = ''
  temp.emergency_contact = ''
  temp.emergency_phone = ''
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
        const res: any = await request.post('/residents', temp)
        if (res.code === 200) {
          ElMessage.success('录入成功')
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
  
  // 处理社区 ID 回填（假设后端没返回 community_id 只有 name，如有需要需特殊处理，这里假定 row 会包含 community_id 或我们在表单需要重新选择）
  if (!temp.community_id && communities.value.length > 0) {
    const matched = communities.value.find(c => c.name === row.community_name)
    if (matched) temp.community_id = matched.id
  }

  dialogStatus.value = 'update'
  dialogFormVisible.value = true
  setTimeout(() => dataFormRef.value?.clearValidate(), 0)
}

const updateData = () => {
  dataFormRef.value?.validate(async (valid) => {
    if (valid) {
      submitLoading.value = true
      try {
        const res: any = await request.put(`/residents/${temp.id}`, temp)
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
  ElMessageBox.confirm('确定要删除该居民档案吗? 此操作不可逆', '警告', {
    confirmButtonText: '确定删除',
    cancelButtonText: '取消',
    type: 'warning'
  }).then(async () => {
    const res: any = await request.delete(`/residents/${row.id}`)
    if (res.code === 200) {
      ElMessage.success('删除成功')
      if (list.value.length === 1 && listQuery.page > 1) {
        listQuery.page--
      }
      getList()
    }
  }).catch(() => {})
}

onMounted(() => {
  getCommunities() // 加载字典
  getList()
})
</script>
