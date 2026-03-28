# 社区健康档案管理系统

> 面向社区健康档案管理场景的课程设计项目，提供居民档案、健康测量、慢病管理与随访管理等能力。

## 1. 项目简介

本项目采用前后端分离架构：

- **后端**：C++17 + cpp-httplib，提供 REST API，并负责静态资源托管  
- **数据库**：MySQL 8.0  
- **前端**：优先使用 Vue3（`vue-frontend`），不可用时回退到旧版页面（`frontend_old`）

主要功能模块：

- 用户登录与权限
- 社区与居民档案管理
- 健康测量与预警
- 慢性病管理
- 随访记录管理

## 2. 前端加载策略（优先 Vue）

服务启动后按以下顺序选择前端资源：

1. 优先挂载 `vue-frontend/dist`（需存在 `index.html`）
2. 若 Vue 产物不存在，回退挂载 `frontend_old`（需存在 `pages/login.html`）
3. 根路径 `/` 跳转规则：
   - Vue 模式：`/ -> /index.html`
   - old frontend 模式：`/ -> /pages/login.html`

即：**优先 Vue，不可用再切换 old frontend**。

## 3. 项目结构

```text
DatabaseCourseDesign/
├── backend/                 # C++ 后端源码
├── vue-frontend/            # Vue3 前端源码（优先）
├── frontend_old/            # 旧版原生前端（回退）
├── sql/                     # 数据库脚本（建表、视图、过程、触发器、权限等）
├── docs/                    # 项目文档
├── Makefile                 # Linux/WSL 常用命令
└── start_linux.sh           # Linux 启动脚本
```

## 4. Linux 使用方法（源码运行）

### 4.1 环境准备

- MySQL 8.0
- CMake
- Git
- C++17 编译器（gcc/clang）
- MySQL 客户端开发库

### 4.2 初始化数据库

在项目根目录执行：

```bash
mysql -u root -p community_health < sql/init.sql
mysql -u root -p community_health < sql/views.sql
mysql -u root -p community_health < sql/procedures.sql
mysql -u root -p community_health < sql/triggers.sql
mysql -u root -p community_health < sql/permissions.sql
```

### 4.3 启动服务（推荐）

前台运行：

```bash
chmod +x ./start_linux.sh
DB_PASS='你的数据库密码' ./start_linux.sh run
```

后台运行：

```bash
DB_PASS='你的数据库密码' ./start_linux.sh start
```

常用管理命令：

```bash
./start_linux.sh status
./start_linux.sh logs
./start_linux.sh health
./start_linux.sh stop
```

默认访问地址：

- 系统入口：`http://127.0.0.1:8080`
- 健康检查：`http://127.0.0.1:8080/api/v1/health-check`

## 5. Windows 使用方法

### 5.1 推荐方式：使用已发布的 Windows Release

你已提供 Windows 发行版，用户侧建议直接使用发行包：

1. 进入项目 **Releases** 页面下载最新版 Windows 包
2. 按发行包内教程完成数据库配置与程序启动

该方式不需要本地编译 C++ 源码，适合演示与快速部署。

### 5.2 从源码运行（可选）

如需在 Windows 本地自行编译，请参考：

- [docs/项目使用说明.md](docs/项目使用说明.md)

其中包含 Windows 本地、WSL 环境下的详细步骤。

## 6. 更多文档

- 使用说明：[docs/项目使用说明.md](docs/项目使用说明.md)
- API 文档：[docs/API接口文档.md](docs/API接口文档.md)
- 数据库设计：[docs/数据库设计文档.md](docs/数据库设计文档.md)
- 项目分工：[docs/项目分工说明.md](docs/项目分工说明.md)

## License

[GPL-3.0](LICENSE)
