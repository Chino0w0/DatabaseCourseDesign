# 社区健康档案管理系统

> 基于前后端分离架构的社区健康档案管理系统课程设计项目。

## 项目说明

本项目当前同时包含三部分：

- `backend/`：C++17 后端服务
- `vue-frontend/`：Vue 3 现代前端
- `frontend_old/`：旧版原生静态前端

其中：

- Linux / WSL / 服务器环境：使用项目根目录的 `start_linux.sh`
- Windows 发布包：只看 `release/` 目录中的说明与脚本

## 技术架构

```text
Vue3 / 原生静态前端  <--HTTP JSON API-->  C++17 后端 (cpp-httplib)
                                           |
                                           +-- MySQL / MariaDB
```

| 层级 | 技术选型 |
|------|---------|
| 前端（现代） | Vue 3 + Vite + TypeScript |
| 前端（旧版） | HTML + JavaScript |
| 后端 | C++17 + cpp-httplib + nlohmann/json |
| 数据库 | MySQL 8.x / MariaDB |
| 构建 | CMake + Make |

## 目录结构

```text
DatabaseCourseDesign/
├── backend/                 # C++ 后端源码
├── vue-frontend/            # Vue 3 前端源码
├── frontend_old/            # 旧版静态前端
├── sql/                     # 数据库初始化脚本
├── docs/                    # 项目文档
├── release/                 # 仅 Windows 发布包说明与脚本
├── start_linux.sh           # Linux / WSL 启动入口
├── Makefile                 # Linux 构建/运行目标
└── README.md                # 当前说明文档
```

## 运行方式总览

### 1. Windows 用户

如果你使用的是已经打包好的 Windows 发布包，请直接阅读：

- `release/README.md`

不要把 `release/` 的启动方式和 Linux 启动方式混用。

### 2. Linux / WSL / 服务器用户

请使用项目根目录的：

- `./start_linux.sh`

本 README 下面的内容主要说明 Linux / WSL 的安装与启动。

---

# Linux / WSL 启动说明

## 一、依赖准备

### 1. 基础工具

至少需要：

- `bash`
- `make`
- `cmake`
- `g++` 或 `clang++`
- `git`（仅当本地第三方头文件不完整时才需要）
- `mysql` 客户端（用于手动初始化数据库时可选）

### 2. MySQL / MariaDB 开发库

后端链接数据库时，需要系统安装 MySQL 或 MariaDB 的开发包。

#### Ubuntu / Debian

推荐安装：

```bash
sudo apt update
sudo apt install -y build-essential cmake make pkg-config default-libmysqlclient-dev mysql-client
```

如果你的系统没有 `default-libmysqlclient-dev`，也可以用：

```bash
sudo apt update
sudo apt install -y build-essential cmake make pkg-config libmariadb-dev mysql-client
```

#### Fedora / RHEL / CentOS Stream

```bash
sudo dnf install -y gcc-c++ cmake make pkgconf-pkg-config mariadb-connector-c-devel mysql
```

#### Arch Linux

```bash
sudo pacman -S --needed base-devel cmake pkgconf mariadb-libs mysql-clients
```

### 3. 前端运行依赖

如果你要运行 Vue 3 开发前端，还需要：

- `node`
- `npm`

Ubuntu / Debian 示例：

```bash
sudo apt install -y nodejs npm
```

如果你只运行后端并使用已经构建好的静态页面，Node 不是必须的。

## 二、数据库初始化

首次使用前，请先准备数据库并导入脚本。

建议执行顺序：

1. `sql/init.sql`
2. `sql/views.sql`
3. `sql/procedures.sql`
4. `sql/triggers.sql`
5. `sql/permissions.sql`
6. `sql/seed.sql`

最基础至少要执行：

- `sql/init.sql`
- `sql/seed.sql`

示例：

```bash
mysql -u root -p < sql/init.sql
mysql -u root -p < sql/views.sql
mysql -u root -p < sql/procedures.sql
mysql -u root -p < sql/triggers.sql
mysql -u root -p < sql/permissions.sql
mysql -u root -p < sql/seed.sql
```

## 三、Linux 启动脚本的目标

`start_linux.sh` 的目标是让 Linux 用户尽量做到：

- 下载项目
- 安装系统依赖
- 导入数据库
- 通过脚本传入参数直接启动

也就是说，数据库连接参数不要求写死到代码里，而是通过脚本参数或环境变量传入。

## 四、最常用启动命令

### 1. 前台启动后端 + 静态页面托管

```bash
./start_linux.sh run --db-host=127.0.0.1 --db-port=3306 --db-user=ch_admin --db-pass='你的数据库密码' --db-name=community_health --port=8080
```

### 2. 后台启动

```bash
./start_linux.sh start --db-host=127.0.0.1 --db-port=3306 --db-user=ch_admin --db-pass='你的数据库密码' --db-name=community_health --port=8080
```

### 3. 仅编译后端

```bash
./start_linux.sh build
```

### 4. 查看状态

```bash
./start_linux.sh status
```

### 5. 查看后台日志

```bash
./start_linux.sh logs
```

### 6. 停止后台服务

```bash
./start_linux.sh stop
```

## 五、脚本支持的参数

`start_linux.sh` 支持以下参数：

- `--db-host=HOST`
- `--db-port=PORT`
- `--db-user=USER`
- `--db-pass=PASSWORD`
- `--db-name=NAME`
- `--port=PORT`
- `--frontend-port=PORT`
- `--help`

也支持通过环境变量传入：

```bash
DB_HOST=127.0.0.1 \
DB_PORT=3306 \
DB_USER=ch_admin \
DB_PASS='你的数据库密码' \
DB_NAME=community_health \
PORT=8080 \
./start_linux.sh run
```

## 六、常用 target

```bash
./start_linux.sh run
./start_linux.sh start
./start_linux.sh stop
./start_linux.sh restart
./start_linux.sh status
./start_linux.sh logs
./start_linux.sh health
./start_linux.sh build
./start_linux.sh clean
./start_linux.sh run-frontend
./start_linux.sh start-frontend
./start_linux.sh run-frontend-old
./start_linux.sh start-frontend-old
```

说明：

- `run` / `start` 会构建后端，并尝试托管前端页面
- `run-frontend` / `start-frontend` 用于单独启动 Vue 3 开发服务
- `run-frontend-old` / `start-frontend-old` 用于单独启动旧版静态前端

## 七、访问地址

默认情况下：

- 后端服务地址：`http://127.0.0.1:8080`
- 健康检查接口：`http://127.0.0.1:8080/api/v1/health-check`
- Vue 开发前端：`http://127.0.0.1:8081`

如果你通过 `--port` 或 `--frontend-port` 传入了新端口，请按实际端口访问。

## 八、常见问题

### 1. CMake 提示找不到 httplib 或 json

本项目优先使用：

- `backend/include/httplib.h`
- `backend/include/json.hpp`

只有当本地头文件缺失时，才会回退到远程拉取依赖。

### 2. CMake 提示找不到 mysqlclient / mariadb

说明系统缺少数据库开发库，请安装：

- Ubuntu / Debian：`default-libmysqlclient-dev` 或 `libmariadb-dev`
- Fedora：`mariadb-connector-c-devel`
- Arch：`mariadb-libs`

### 3. 启动脚本提示必须提供数据库密码

这是刻意设计的安全限制。

如果 target 会启动后端服务，你需要显式传入：

- `--db-pass='你的密码'`

或者先设置环境变量：

```bash
export DB_PASS='你的密码'
```

### 4. 启动后浏览器能打开但没有页面

后端会优先挂载：

1. `vue-frontend/dist`
2. `frontend_old`

如果你没有构建 Vue 产物，又不想使用旧版页面，请先进入 `vue-frontend/` 构建前端。

## 九、开发说明

- Linux / WSL 启动：`start_linux.sh`
- Windows 发布包运行：`release/README.md`
- 后端构建入口：`backend/CMakeLists.txt`
- Linux 构建与运行目标：`Makefile`

## 十、License

- `GPL-3.0`
