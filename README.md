# 社区健康档案管理系统

> 基于前后端分离架构的社区健康档案管理系统课程设计项目。

---

## 目录

- [项目简介](#项目简介)
- [功能概览](#功能概览)
- [技术架构](#技术架构)
- [目录结构](#目录结构)
- [默认账号](#默认账号)
- [数据库初始化](#数据库初始化)
- [Linux / WSL 使用说明](#linux--wsl-使用说明)
  - [系统依赖安装](#系统依赖安装)
  - [启动与停止](#启动与停止)
  - [常用命令速查](#常用命令速查)
  - [访问地址](#访问地址)
- [Windows 使用说明](#windows-使用说明)
  - [原生 Windows（MSYS2 / MinGW）](#原生-windowsmsys2--mingw)
  - [WSL（推荐）](#wsl推荐)
- [常见问题](#常见问题)
- [API 说明](#api-说明)
- [License](#license)

---

## 项目简介

本系统是面向社区卫生管理场景的健康档案信息化平台，支持管理员、医生、护士与居民四类角色，覆盖居民档案管理、健康体检记录、慢性病追踪与随访管理等核心业务流程。

系统采用前后端分离设计：

- 后端以 **C++17 + cpp-httplib** 构建 HTTP JSON API，并同时托管前端静态资源
- 前端以 **Vue 3 + Vite + TypeScript** 实现，打包后由后端直接伺服
- 数据库使用 **MySQL 8.x / MariaDB**

---

## 功能概览

### 所有角色通用
| 功能 | 说明 |
|------|------|
| 登录 / 登出 | 基于 Token 的会话管理 |
| 修改自己的密码 | 修改成功后当前会话立即失效，需重新登录 |

### 管理员（admin）
| 功能 | 说明 |
|------|------|
| 数据概览 | 系统整体统计指标与快捷操作向导 |
| 居民管理 | 新增 / 查询 / 编辑 / 删除居民档案与社区信息 |
| 健康档案 | 查询健康档案、录入 / 编辑体检记录、查看异常预警 |
| 随访管理 | 查询 / 新增 / 编辑随访记录 |
| 系统管理 | 新增 / 编辑 / 启用禁用系统账户；重置用户密码（被重置用户将被强制下线） |

### 医生 / 护士
| 功能 | 说明 |
|------|------|
| 数据概览 | 查看整体统计 |
| 居民管理 | 查询居民档案（无删除权限） |
| 健康档案 | 录入 / 编辑体检记录、查看异常预警 |
| 随访管理 | 新增 / 编辑随访记录 |

### 居民
| 功能 | 说明 |
|------|------|
| 我的健康档案 | 仅查看本人健康测量记录与健康摘要信息 |

---

## 技术架构

```text
Vue3 前端 (Vite 开发服务 / dist 静态产物)
        │
        │  HTTP JSON API
        ▼
C++17 后端 (cpp-httplib)  ←→  MySQL 8.x / MariaDB
```

| 层级 | 技术选型 |
|------|---------|
| 现代前端 | Vue 3 + Vite + TypeScript + Element Plus |
| 旧版前端（备用） | HTML + JavaScript（托管于 `frontend_old/`） |
| 后端 | C++17 + cpp-httplib + nlohmann/json |
| 数据库 | MySQL 8.x / MariaDB |
| 构建 | CMake + Make |
| 认证 | Token（SHA-256，内存 Session 管理） |

---

## 目录结构

```text
DatabaseCourseDesign/
├── backend/                 # C++ 后端源码
│   ├── controllers/         # HTTP 路由处理
│   ├── services/            # 业务逻辑
│   ├── dao/                 # 数据库访问
│   ├── models/              # 数据模型
│   ├── utils/               # 工具类（会话、数据库、响应封装）
│   ├── include/             # 第三方头文件（httplib、json）
│   ├── tests/               # 后端单元 / 集成测试
│   ├── main.cpp             # 服务启动入口
│   └── CMakeLists.txt       # 后端构建配置
├── vue-frontend/            # Vue 3 前端源码
│   ├── src/
│   │   ├── layout/          # 整体布局（含顶栏菜单与修改密码弹窗）
│   │   ├── views/           # 各业务页面
│   │   ├── router/          # 前端路由
│   │   ├── store/           # 用户状态
│   │   └── utils/           # 请求封装
│   └── vite.config.ts       # Vite 构建配置
├── frontend_old/            # 旧版原生静态前端（备用）
├── sql/                     # 数据库脚本
│   ├── init.sql             # 建表 + 默认管理员账号
│   ├── views.sql            # 视图
│   ├── procedures.sql       # 存储过程
│   ├── triggers.sql         # 触发器
│   ├── permissions.sql      # 数据库账户权限
│   └── seed.sql             # 测试种子数据（医生、护士、居民示例账号）
├── docs/                    # 项目文档
├── Makefile                 # Linux 构建 / 运行目标
├── start_linux.sh           # Linux / WSL 一键启动脚本
└── README.md                # 本文档
```

---

## 默认账号

执行 `sql/init.sql` 后自动创建管理员账号，执行 `sql/seed.sql` 后额外创建以下测试账号（密码均为 `123456`）：

| 用户名 | 角色 | 说明 |
|--------|------|------|
| `admin` | 管理员 | 默认管理员，`sql/init.sql` 创建 |
| `doctor_zhang` | 医生 | 张伟医生 |
| `doctor_li` | 医生 | 李华医生 |
| `nurse_wang` | 护士 | 王芳护士 |
| `resident_zhangsan` | 居民 | 居民自助账号（绑定张三档案） |

---

## 数据库初始化

首次使用前请依次执行以下 SQL 脚本：

```bash
# 1. 建表 + 角色 + 默认管理员账号
mysql -u root -p < sql/init.sql

# 2. 视图
mysql -u root -p < sql/views.sql

# 3. 存储过程
mysql -u root -p < sql/procedures.sql

# 4. 触发器
mysql -u root -p < sql/triggers.sql

# 5. 数据库账户权限（用于 ch_admin 账号）
mysql -u root -p < sql/permissions.sql

# 6. 测试种子数据（可选，推荐演示时使用）
mysql -u root -p < sql/seed.sql
```

> **最少执行**：`init.sql` + `permissions.sql`，如需完整演示请执行全部 6 个脚本。

---

## Linux / WSL 使用说明

### 系统依赖安装

#### Ubuntu / Debian

```bash
sudo apt update
sudo apt install -y build-essential cmake make pkg-config \
    default-libmysqlclient-dev mysql-client git
```

如需运行 Vue 3 开发前端，还需要 Node.js：

```bash
sudo apt install -y nodejs npm
```

#### Fedora / RHEL / CentOS Stream

```bash
sudo dnf install -y gcc-c++ cmake make pkgconf-pkg-config \
    mariadb-connector-c-devel mysql
```

#### Arch Linux

```bash
sudo pacman -S --needed base-devel cmake pkgconf mariadb-libs mysql-clients
```

---

### 启动与停止

项目提供两个等价的入口：

| 入口 | 适用场景 |
|------|---------|
| [`./start_linux.sh`](start_linux.sh) | 更友好的参数解析，推荐日常使用 |
| `make <target>` | 直接调用 Makefile 目标 |

#### 1. 前台运行（推荐演示时使用，Ctrl+C 停止）

```bash
./start_linux.sh run --db-pass='你的数据库密码'
```

#### 2. 后台运行

```bash
./start_linux.sh start --db-pass='你的数据库密码'
```

如果数据库密码为空或需要自定义全部参数：

```bash
./start_linux.sh start \
  --db-host=127.0.0.1 \
  --db-port=3306 \
  --db-user=ch_admin \
  --db-pass='你的密码' \
  --db-name=community_health \
  --port=8080
```

也可以通过环境变量传入：

```bash
export DB_PASS='你的密码'
./start_linux.sh start
```

#### 3. 停止服务

```bash
./start_linux.sh stop
```

#### 4. 重启服务

```bash
./start_linux.sh restart --db-pass='你的密码'
```

#### 5. 仅编译后端，不启动服务

```bash
./start_linux.sh build
```

---

### 常用命令速查

| 命令 | 说明 |
|------|------|
| `./start_linux.sh run` | 前台启动后端 + 静态页面托管 |
| `./start_linux.sh start` | 后台启动 |
| `./start_linux.sh stop` | 停止后台服务 |
| `./start_linux.sh restart` | 重启后台服务 |
| `./start_linux.sh status` | 查看服务状态 |
| `./start_linux.sh logs` | 查看后台日志 |
| `./start_linux.sh health` | 调用健康检查接口 |
| `./start_linux.sh build` | 仅编译后端 |
| `./start_linux.sh clean` | 清理构建目录与运行缓存 |
| `./start_linux.sh run-frontend` | 单独前台启动 Vue 3 开发服务（需要 Node.js） |
| `./start_linux.sh start-frontend` | 单独后台启动 Vue 3 开发服务 |
| `./start_linux.sh run-frontend-old` | 单独前台启动旧版静态前端（需要 Python 3） |

---

### 访问地址

| 地址 | 说明 |
|------|------|
| `http://127.0.0.1:8080` | 系统入口（默认端口） |
| `http://127.0.0.1:8080/api/v1/health-check` | 后端健康检查 |
| `http://127.0.0.1:8081` | Vue 3 开发服务（`run-frontend` 时使用） |

> 如果你通过 `--port` 指定了不同端口，请将 `8080` 替换为实际端口。

---

## Windows 使用说明

### 原生 Windows（MSYS2 / MinGW）

#### 1. 安装依赖

从 [msys2.org](https://www.msys2.org/) 安装 MSYS2，然后在 UCRT64 Shell 中执行：

```bash
pacman -S --needed mingw-w64-ucrt-x86_64-gcc \
    mingw-w64-ucrt-x86_64-cmake \
    mingw-w64-ucrt-x86_64-ninja \
    mingw-w64-ucrt-x86_64-libmariadbclient \
    git
```

#### 2. 数据库初始化

参考 [数据库初始化](#数据库初始化) 章节，使用 MySQL Workbench / Navicat / DBeaver 等工具手动执行 SQL 脚本，或使用命令行：

```bash
mysql -u root -p < sql/init.sql
mysql -u root -p < sql/permissions.sql
# ... 其余脚本按需执行
```

#### 3. 编译后端

在 MSYS2 UCRT64 Shell 中，进入项目根目录执行：

```bash
cmake -S backend -B backend/build -G Ninja
cmake --build backend/build -j 4
```

如果 CMake 找不到 MySQL / MariaDB 开发库，需要手动指定：

```bash
cmake -S backend -B backend/build -G Ninja \
  -DMYSQLCLIENT_INCLUDE_DIR="C:/msys64/ucrt64/include/mariadb" \
  -DMYSQLCLIENT_LIBRARY="C:/msys64/ucrt64/lib/libmariadb.dll.a"
cmake --build backend/build -j 4
```

#### 4. 启动服务

**PowerShell：**

```powershell
$env:DB_HOST = "127.0.0.1"
$env:DB_PORT = "3306"
$env:DB_USER = "ch_admin"
$env:DB_PASS = "你的数据库密码"
$env:DB_NAME = "community_health"
.\backend\build\community_health_server.exe
```

**CMD：**

```cmd
set DB_HOST=127.0.0.1
set DB_PORT=3306
set DB_USER=ch_admin
set DB_PASS=你的数据库密码
set DB_NAME=community_health
backend\build\community_health_server.exe
```

#### 5. 访问系统

浏览器打开：

```text
http://127.0.0.1:8080
```

---

### WSL（推荐）

如果你的 Windows 已安装 WSL（Ubuntu / Debian 推荐），可以在 WSL 中像 Linux 一样使用本项目，体验更一致。

#### 1. 进入项目目录

```bash
# 假设项目在 Windows 的 D: 盘
cd /mnt/d/DatabaseCourseDesign
```

#### 2. 安装依赖

```bash
sudo apt update
sudo apt install -y build-essential cmake make pkg-config \
    default-libmysqlclient-dev mysql-client git
```

#### 3. 初始化数据库 & 启动服务

与 [Linux / WSL 使用说明](#linux--wsl-使用说明) 完全一致：

```bash
chmod +x ./start_linux.sh
./start_linux.sh run --db-pass='你的密码'
```

#### 4. 在 Windows 终端中调用 WSL 启动

```cmd
wsl bash -lc "cd /mnt/d/DatabaseCourseDesign && ./start_linux.sh start --db-pass='你的密码'"
```

---

## 常见问题

### 浏览器无法访问 `127.0.0.1:8080`

- 确认后端服务已启动：`./start_linux.sh status`
- 确认端口未被占用：`fuser 8080/tcp`
- 查看启动日志：`./start_linux.sh logs`

### 登录失败 / 提示密码错误

- 确认已执行 `sql/init.sql`
- 默认管理员账号为 `admin / 123456`
- 确认环境变量中的数据库配置指向正确的数据库实例

### 后端提示数据库连接失败

- 确认 MySQL 服务已启动
- 确认 `DB_HOST`、`DB_PORT`、`DB_USER`、`DB_PASS`、`DB_NAME` 正确
- 如 `127.0.0.1` 连不上，可尝试改为 `localhost`，反之亦然
- 确认 `sql/permissions.sql` 已执行（`ch_admin` 账号权限）

### CMake 提示找不到 mysqlclient / mariadb

```bash
# Ubuntu / Debian
sudo apt install -y default-libmysqlclient-dev
# 或
sudo apt install -y libmariadb-dev

# Fedora
sudo dnf install -y mariadb-connector-c-devel

# Arch
sudo pacman -S mariadb-libs
```

### 服务启动成功但页面空白或 404

后端会按以下优先级挂载前端静态文件：

1. `vue-frontend/dist`（Vue 3 生产产物）
2. `frontend_old`（旧版静态前端）

如果两者都不存在，则所有非 API 路径均返回 404。请先构建前端：

```bash
./start_linux.sh build-frontend
# 或手动
cd vue-frontend && npm install && npm run build
```

---

## API 说明

详细接口文档见 [`docs/API接口文档.md`](docs/API接口文档.md)。

主要接口前缀：

| 前缀 | 模块 |
|------|------|
| `POST /api/v1/auth/login` | 用户登录 |
| `POST /api/v1/auth/logout` | 用户登出 |
| `GET/POST /api/v1/users` | 用户管理（仅管理员） |
| `PUT /api/v1/users/:id` | 编辑用户（仅管理员） |
| `PUT /api/v1/users/me/password` | 当前用户修改自己的密码 |
| `GET/POST /api/v1/communities` | 社区列表 |
| `GET/POST/PUT/DELETE /api/v1/residents` | 居民档案 CRUD |
| `GET/POST/PUT /api/v1/health` | 健康体检记录 |
| `GET/POST/PUT/DELETE /api/v1/visits` | 随访记录 |
| `GET/POST /api/v1/diseases` | 慢性病信息 |
| `GET /api/v1/my-health` | 居民查看本人健康档案 |
| `GET /api/v1/health-check` | 服务健康检查 |

---

## License

本项目基于 [GPL-3.0](LICENSE) 协议开源。

