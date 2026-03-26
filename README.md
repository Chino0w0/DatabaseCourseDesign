# 社区健康档案管理系统

> 基于前后端分离架构的社区健康档案管理系统 — 数据库课程设计项目

## 技术架构

```
Web 前端 (HTML/JS) ←──[HTTP REST API / JSON]──→ C++ 后端 (cpp-httplib) ←──[SQL]──→ MySQL
```

| 层级 | 技术选型 |
|------|---------|
| **前端** | HTML5 + 原生 JavaScript (Fetch API) |
| **后端** | C++ 17 + cpp-httplib + nlohmann/json |
| **数据库** | MySQL 8.0 |

## 项目结构

```
DatabaseCourseDesign/
├── docs/                          # 📄 项目文档
│   ├── 项目分工说明.md             # 按模块划分的开发分工
│   ├── 课程设计要求.md             # 课程设计要求 & 非程序任务
│   ├── API接口文档.md              # RESTful API 规范
│   └── 数据库设计文档.md           # 表结构 & ER 图
├── sql/                           # 🗄️ 数据库脚本
│   ├── init.sql                   # 建表 + 初始数据
│   ├── views.sql                  # 查询视图
│   ├── procedures.sql             # 存储过程
│   ├── triggers.sql               # 触发器
│   ├── permissions.sql            # 用户权限
│   └── seed.sql                   # 测试种子数据
├── backend/                       # ⚙️ C++ 后端
│   ├── include/                   # 第三方头文件
│   ├── models/                    # 实体层
│   ├── dao/                       # 数据访问层
│   ├── services/                  # 业务逻辑层
│   ├── controllers/               # 路由接口层
│   ├── utils/                     # 工具类
│   ├── main.cpp                   # 服务器入口
│   └── CMakeLists.txt             # CMake 构建
└── frontend/                      # 🎨 Web 前端
    ├── css/                       # 样式
    ├── js/                        # 脚本
    └── pages/                     # 页面
```

## 快速开始

### 1. 初始化数据库

```bash
mysql -u root -p < sql/init.sql
```

### 2. 编译后端

```bash
cd backend
cmake -B build
cmake --build build
./build/community_health_server
```

### 3. 打开前端

浏览器访问 `frontend/pages/login.html` 或通过后端静态文件服务访问。

## 分工说明

详见 [docs/项目分工说明.md](docs/项目分工说明.md)

## License

[GPL-3.0](LICENSE)
