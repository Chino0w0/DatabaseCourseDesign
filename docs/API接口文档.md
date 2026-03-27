# 社区健康档案管理系统 — RESTful API 接口文档

> **Base URL**：`http://localhost:8080/api/v1`  
> **数据格式**：JSON (`Content-Type: application/json`)

---

## 统一响应格式

```json
{
  "code": 200,          // 状态码：200 成功, 400 参数错误, 401 未认证, 500 服务器错误
  "msg": "操作成功",     // 提示信息
  "data": { ... }       // 业务数据（可为 null）
}
```

---

## 一、认证模块 (`/api/v1/auth`)

### 1.1 用户登录

| 项目 | 说明 |
|------|------|
| **URL** | `POST /api/v1/auth/login` |
| **描述** | 验证用户名和密码 |

**请求体 (Request Body)**：

```json
{
  "username": "admin",
  "password": "123456"
}
```

**成功响应 (200)**：

```json
{
  "code": 200,
  "msg": "登录成功",
  "data": {
    "user_id": 1,
    "username": "admin",
    "role": "管理员"
  }
}
```

**失败响应 (401)**：

```json
{
  "code": 401,
  "msg": "用户名或密码错误",
  "data": null
}
```

---

## 二、用户管理模块 (`/api/v1/users`)

### 2.1 获取用户列表

| 项目 | 说明 |
|------|------|
| **URL** | `GET /api/v1/users` |
| **描述** | 获取所有系统用户（管理员权限） |
| **查询参数** | `?page=1&size=10` |

### 2.2 创建用户

| 项目 | 说明 |
|------|------|
| **URL** | `POST /api/v1/users` |
| **描述** | 新增系统用户 |

```json
{
  "username": "doctor01",
  "password": "123456",
  "real_name": "张医生",
  "role_id": 2
}
```

### 2.3 修改用户

| 项目 | 说明 |
|------|------|
| **URL** | `PUT /api/v1/users/{id}` |
| **描述** | 修改用户信息 |

### 2.4 删除用户

| 项目 | 说明 |
|------|------|
| **URL** | `DELETE /api/v1/users/{id}` |
| **描述** | 删除用户（软删除） |

---

## 三、居民管理模块 (`/api/v1/residents`)

### 3.1 获取居民列表

| 项目 | 说明 |
|------|------|
| **URL** | `GET /api/v1/residents` |
| **描述** | 获取居民列表（支持分页和搜索） |
| **查询参数** | `?page=1&size=10&keyword=张三&community_id=1` |

**成功响应**：

```json
{
  "code": 200,
  "msg": "查询成功",
  "data": {
    "total": 100,
    "page": 1,
    "size": 10,
    "list": [
      {
        "id": 1,
        "name": "张三",
        "gender": "男",
        "id_card": "110101199001011234",
        "phone": "13800138000",
        "birth_date": "1990-01-01",
        "community_name": "阳光社区",
        "address": "XX路XX号"
      }
    ]
  }
}
```

### 3.2 获取居民详情

| 项目 | 说明 |
|------|------|
| **URL** | `GET /api/v1/residents/{id}` |
| **描述** | 获取单个居民的完整信息 |

### 3.3 新增居民

| 项目 | 说明 |
|------|------|
| **URL** | `POST /api/v1/residents` |
| **描述** | 新增居民档案 |

```json
{
  "name": "张三",
  "gender": "男",
  "id_card": "110101199001011234",
  "phone": "13800138000",
  "birth_date": "1990-01-01",
  "community_id": 1,
  "address": "XX路XX号",
  "emergency_contact": "李四",
  "emergency_phone": "13900139000"
}
```

### 3.4 修改居民信息

| 项目 | 说明 |
|------|------|
| **URL** | `PUT /api/v1/residents/{id}` |
| **描述** | 更新居民信息 |

### 3.5 删除居民

| 项目 | 说明 |
|------|------|
| **URL** | `DELETE /api/v1/residents/{id}` |
| **描述** | 删除居民档案 |

---

## 四、健康测量模块 (`/api/v1/health`)

### 4.1 获取居民健康测量记录

| 项目 | 说明 |
|------|------|
| **URL** | `GET /api/v1/health/measurements?resident_id=1` |
| **描述** | 查询某位居民的历史测量数据 |
| **可选参数** | `limit`（默认 50，最大 200） |

**成功响应**：

```json
{
  "code": 200,
  "msg": "查询成功",
  "data": [
    {
      "id": 1,
      "resident_id": 1,
      "systolic": 135,
      "diastolic": 88,
      "blood_sugar": 5.6,
      "heart_rate": 72,
      "height": 172.0,
      "weight": 68.0,
      "bmi": 23.0,
      "notes": "晨起空腹测量",
      "measured_by_user_id": 2,
      "measured_at": "2026-03-25 10:30:00",
      "measured_by": "张医生",
      "warning": true,
      "warning_msg": ["高血压：收缩压: 135 mmHg, 舒张压: 88 mmHg"]
    }
  ]
}
```

### 4.2 录入健康测量数据

| 项目 | 说明 |
|------|------|
| **URL** | `POST /api/v1/health/measurements` |
| **描述** | 新增一条测量记录 |

```json
{
  "resident_id": 1,
  "systolic": 135,
  "diastolic": 88,
  "blood_sugar": 5.6,
  "heart_rate": 72,
  "measured_by_user_id": 2
}
```

### 4.3 获取健康档案摘要

| 项目 | 说明 |
|------|------|
| **URL** | `GET /api/v1/health/records/{resident_id}` |
| **描述** | 获取居民的健康档案摘要 |

**成功响应**：

```json
{
  "code": 200,
  "msg": "查询成功",
  "data": {
    "id": 10,
    "resident_id": 1,
    "blood_type": "A",
    "allergy_history": null,
    "family_history": null,
    "past_medical_history": null,
    "created_at": "2026-03-20 09:00:00",
    "updated_at": "2026-03-26 18:00:00"
  }
}
```

---

## 五、慢性病管理模块 (`/api/v1/diseases`)

### 5.1 获取慢性病字典

| 项目 | 说明 |
|------|------|
| **URL** | `GET /api/v1/diseases` |
| **描述** | 获取所有慢性病类型 |

### 5.2 为居民关联慢性病

| 项目 | 说明 |
|------|------|
| **URL** | `POST /api/v1/residents/{id}/diseases` |
| **描述** | 为某居民添加慢性病诊断记录 |

```json
{
  "disease_id": 1,
  "diagnosed_date": "2024-06-15",
  "notes": "需长期服药控制"
}
```

### 5.3 获取居民慢性病列表

| 项目 | 说明 |
|------|------|
| **URL** | `GET /api/v1/residents/{id}/diseases` |
| **描述** | 查看某居民的慢性病列表 |

---

## 六、随访管理模块 (`/api/v1/visits`)

### 6.1 获取随访记录

| 项目 | 说明 |
|------|------|
| **URL** | `GET /api/v1/visits?resident_id=1` |
| **描述** | 查询某居民的随访记录 |

### 6.2 新增随访记录

| 项目 | 说明 |
|------|------|
| **URL** | `POST /api/v1/visits` |
| **描述** | 新增随访记录 |

```json
{
  "resident_id": 1,
  "visit_type": "上门随访",
  "visit_date": "2026-03-26",
  "content": "血压复查，情况稳定",
  "visitor_user_id": 2,
  "next_visit_date": "2026-04-26"
}
```

---

## 七、社区管理模块 (`/api/v1/communities`)

### 7.1 获取社区列表

| 项目 | 说明 |
|------|------|
| **URL** | `GET /api/v1/communities` |
| **描述** | 获取所有社区 |

### 7.2 新增社区

| 项目 | 说明 |
|------|------|
| **URL** | `POST /api/v1/communities` |
| **描述** | 新增社区 |

```json
{
  "name": "阳光社区",
  "address": "XX区XX街道",
  "contact_phone": "010-12345678"
}
```
