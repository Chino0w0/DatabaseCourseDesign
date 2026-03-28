# 社区健康档案管理系统

> 基于前后端分离架构的社区健康档案管理系统（数据库课程设计）

## 技术架构

```text
前端（Vue3 / 旧版原生页面） ←── HTTP REST API / JSON ──→ C++ 后端（cpp-httplib） ←── SQL ──→ MySQL
```

| 层级 | 技术选型 |
|------|---------|
| **前端（优先）** | Vue 3 + TypeScript + Vite（`vue-frontend`） |
| **前端（回退）** | 原生 HTML/CSS/JS（`frontend_old`） |
| **后端** | C++17 + cpp-httplib + nlohmann/json |
| **数据库** | MySQL 8.0 |

## 当前前端启动逻辑（你关心的“优先 Vue，失败再回退”）

后端启动后会自动挂载静态资源，规则如下：

1. **优先尝试 Vue 产物**：查找 `vue-frontend/dist/index.html`
2. 如果 Vue 产物不存在，再尝试 **old frontend**：查找 `frontend_old/pages/login.html`
3. 根路径 `/` 行为：
   - 挂载 Vue 时：`/ -> /index.html`
   - 挂载 old frontend 时：`/ -> /pages/login.html`

也就是说：**当前已经是“优先 Vue，不能用 Vue 再切 old frontend”**。

> 注意：仓库当前 `vue-frontend` 目录缺少 `package.json`，因此在当前仓库态下通常会回退到 `frontend_old`。  
> 为避免启动失败，`make build-frontend` 已做防护：检测不到 `package.json` 时会跳过 Vue 构建并继续后端启动。

## 项目结构

```text
DatabaseCourseDesign/
├── backend/                 # C++ 后端
├── vue-frontend/            # Vue3 前端源码（优先）
├── frontend_old/            # 旧版原生前端（回退）
├── sql/                     # 数据库脚本
├── docs/                    # 项目文档
├── Makefile                 # Linux/WSL 常用构建与启动命令
└── start_linux.sh           # Linux 启动脚本封装
```

## Linux 使用方法（源码运行）

### 1) 初始化数据库

```bash
mysql -u root -p community_health < sql/init.sql
mysql -u root -p community_health < sql/views.sql
mysql -u root -p community_health < sql/procedures.sql
mysql -u root -p community_health < sql/triggers.sql
mysql -u root -p community_health < sql/permissions.sql
```

### 2) 启动（推荐）

```bash
chmod +x ./start_linux.sh
DB_PASS='你的数据库密码' ./start_linux.sh run
```

后台运行：

```bash
DB_PASS='你的数据库密码' ./start_linux.sh start
```

查看状态 / 日志 / 健康检查：

```bash
./start_linux.sh status
./start_linux.sh logs
./start_linux.sh health
```

停止：

```bash
./start_linux.sh stop
```

## Windows 使用方法

你已提供 Windows Release 发行版，**Windows 端请优先按发行包内教程操作**（无需本地编译源码）。

- 推荐入口：`Releases` 页面下载你发布的 Windows 发行包
- 运行步骤：以发行包内文档/教程为准

如需从源码运行，可参考文档：
- [docs/项目使用说明.md](docs/项目使用说明.md)（含 Windows 本地与 WSL 说明）

## 更多文档

- 项目分工：[docs/项目分工说明.md](docs/项目分工说明.md)
- API 文档：[docs/API接口文档.md](docs/API接口文档.md)
- 数据库设计：[docs/数据库设计文档.md](docs/数据库设计文档.md)
- 使用说明：[docs/项目使用说明.md](docs/项目使用说明.md)

## License

[GPL-3.0](LICENSE)
