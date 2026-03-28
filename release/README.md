# 社区健康档案管理系统 - Windows 发布包使用说明

本目录仅用于 **Windows 发布包** 场景。

如果你是在 Linux / WSL / 服务器环境下启动源码项目，请不要使用本目录中的脚本，而应回到项目根目录使用：

- `start_linux.sh`
- `README.md`

本发布包的目标是让 Windows 用户 **不需要安装 Node / npm / CMake / WSL / C++ 编译器**，只需准备好 MySQL，并按步骤配置后即可运行系统。

## 1. 运行前需要准备什么

你需要具备以下条件：

- Windows 系统
- 一个可访问的 MySQL 8.x 或兼容 MariaDB 实例
- 已导入本项目 SQL 初始化脚本
- 已准备好发布包中的后端程序与配置文件

你的 MySQL 可以来自两种常见方式：

### 方式 A：本机直接安装 MySQL

这是最常规的方式。

### 方式 B：Docker 中运行 MySQL

也是可行的。

只要 Docker 容器把数据库端口映射到了宿主机，例如：

```text
127.0.0.1:3306 -> 容器内 3306
```

那么本发布包依然可以正常连接。

## 2. 发布包目录说明

建议拿到发布包后保持目录结构不变。关键文件如下：

- `release/app/community_health_server.exe`：后端主程序
- `release/config/app.env.example`：配置模板
- `release/config/app.env`：实际运行配置
- `release/scripts/start_windows.bat`：Windows 启动脚本
- `release/scripts/stop_windows.bat`：Windows 停止脚本

## 3. 初始化数据库

首次运行前，请先创建数据库并导入 SQL。

建议执行顺序：

1. `sql/init.sql`
2. `sql/views.sql`
3. `sql/procedures.sql`
4. `sql/triggers.sql`
5. `sql/permissions.sql`
6. `sql/seed.sql`

如果你只做最基础初始化，也至少应完成：

- `sql/init.sql`
- `sql/seed.sql`

## 4. 配置数据库连接

先复制配置模板：

```cmd
copy release\config\app.env.example release\config\app.env
```

然后打开 `release/config/app.env`，按你的实际环境填写：

```env
DB_HOST=127.0.0.1
DB_PORT=3306
DB_USER=ch_admin
DB_PASS=请改成你的数据库密码
DB_NAME=community_health
PORT=8080
```

说明：

- `DB_PASS` 没有安全默认值，必须按实际环境填写
- 如果数据库不在本机，请把 `DB_HOST` 改成目标机器 IP
- 如果 Docker 映射端口不是 `3306`，请同步修改 `DB_PORT`
- 如果你修改了数据库名 / 账号 / 密码，这里也要保持一致

## 5. 启动系统

确认数据库已启动、脚本已导入、配置文件已填写后，直接双击：

- `release/scripts/start_windows.bat`

启动成功后，在浏览器访问：

```text
http://127.0.0.1:8080
```

如果你在 `app.env` 中改了 `PORT`，请按实际端口访问。

## 6. 关于默认账号

本项目源码中可能包含课程设计阶段的演示账号或测试数据，但 **Windows 发布包使用时不应依赖这些默认口令**。

建议你在导入数据库后立即完成以下动作：

1. 修改数据库连接账号密码
2. 修改应用登录账号密码
3. 不要在生产或演示机器上继续保留公开文档中的初始口令

## 7. 常见问题

### 7.1 启动后提示数据库连接失败

优先检查：

- MySQL 是否已启动
- `release/config/app.env` 中的 `DB_HOST` 是否正确
- `DB_PORT` 是否正确
- `DB_USER` / `DB_PASS` 是否正确
- `DB_NAME` 是否存在
- 数据库脚本是否已成功导入

### 7.2 Docker MySQL 已启动，但程序连不上

通常是以下原因：

- Docker 没做端口映射
- 映射端口不是 `3306`，但配置里仍写成了 `3306`
- 用户权限只允许容器内访问，不允许宿主机访问
- 数据库尚未初始化完成

### 7.3 浏览器能打开，但页面没有数据

通常说明：

- 后端已启动
- 但数据库中缺少初始化数据

请确认是否至少执行了：

- `sql/init.sql`
- `sql/seed.sql`

### 7.4 我需要安装 Node 或 npm 吗

不需要。

本发布包的目标就是让你直接运行已构建产物，而不是在本机重新构建 `vue-frontend/`。

### 7.5 我需要安装 WSL 吗

不需要。

WSL 只属于开发阶段，不是 Windows 发布包的运行前提。

## 8. 面向使用者的最短说明

如果你已经了解 MySQL，最短步骤如下：

1. 准备好 MySQL（本机或 Docker 均可）
2. 导入 `sql/init.sql` 和 `sql/seed.sql`
3. 复制 `release/config/app.env.example` 为 `release/config/app.env`
4. 填写真实数据库连接参数
5. 双击 `release/scripts/start_windows.bat`
6. 打开 `http://127.0.0.1:8080`

## 9. 说明边界

- `release/`：仅 Windows 发布包说明与脚本
- 项目根目录 `README.md`：Linux / WSL / 源码运行说明
- 项目根目录 `start_linux.sh`：Linux 启动入口
