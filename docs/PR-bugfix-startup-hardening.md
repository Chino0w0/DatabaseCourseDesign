# PR：修复启动链路并加固跨平台运行说明

## 一、背景

本次修改主要围绕以下问题展开：

1. Linux 启动链路中存在本地依赖优先级不清、数据库密码默认值不安全、脚本参数能力不足的问题。
2. 后端服务端口与脚本传参不一致，导致脚本可配置但服务端实际仍固定监听默认端口。
3. Vue 前端与旧版静态前端的入口逻辑混用，存在根路径跳转错误风险。
4. Windows 发布包说明与源码运行说明边界不清。
5. 部分控制器直接将内部异常透出到前端，存在错误信息暴露问题。
6. 示例配置与权限脚本中包含真实敏感默认值，不适合继续作为共享仓库默认内容。

## 二、主要修改内容

### 1. 启动与构建链路修复

- 调整 `backend/CMakeLists.txt`，优先使用本地 `backend/include/` 中的第三方头文件，仅在缺失时再回退到远程依赖拉取。
- 调整 `backend/main.cpp`：
  - 服务端口改为读取环境变量 `PORT`
  - 按前端类型区分入口逻辑
  - Vue 产物挂载时走 `/index.html`
  - 旧版静态前端挂载时走 `/pages/login.html`
- 调整 `start_linux.sh`：
  - 作为 Linux / WSL 独立启动入口
  - 支持命令行传入数据库参数和端口参数
  - 不再内置真实数据库密码
  - 对必须启动后端的 target 强制要求显式提供 `DB_PASS`

### 2. 文档与运行边界梳理

- 重写项目根目录 `README.md`
  - 明确 Linux / WSL / 服务器场景使用方式
  - 增补 MySQL / MariaDB 开发包安装说明
  - 说明如何通过脚本传参直接启动
- 重写 `release/README.md`
  - 明确 `release/` 仅服务于 Windows 发布包
  - 与源码运行说明分离
  - 补充配置与常见问题说明

### 3. Windows 发布包启动稳健性修复

- 修复 `release/scripts/start_windows.bat` 的配置解析方式
- 使用 `DisableDelayedExpansion`，避免包含 `!` 的密码被错误展开
- 增加 `DB_PASS` 缺失时的显式报错

### 4. 示例敏感信息清理

- 更新 `release/config/app.env.example`，去除真实默认密码
- 更新 `sql/permissions.sql`，改为占位密码并要求手动替换
- 更新 `backend/tests/test_all.cpp` 与 `docs/项目使用说明.md` 中的数据库密码示例为占位值

### 5. 错误回显收敛

- 调整以下控制器，避免把 `e.what()` 直接返回前端：
  - `backend/controllers/AuthController.cpp`
  - `backend/controllers/UserController.cpp`
  - `backend/controllers/HealthController.cpp`
  - `backend/controllers/DiseaseController.cpp`
  - `backend/controllers/VisitController.cpp`
  - `backend/controllers/ResidentController.cpp`
- 改为：
  - 服务端日志记录详细异常
  - 前端仅收到通用错误提示文案

## 三、影响范围

### 后端

- `backend/CMakeLists.txt`
- `backend/main.cpp`
- `backend/controllers/AuthController.cpp`
- `backend/controllers/UserController.cpp`
- `backend/controllers/HealthController.cpp`
- `backend/controllers/DiseaseController.cpp`
- `backend/controllers/VisitController.cpp`
- `backend/controllers/ResidentController.cpp`
- `backend/tests/test_all.cpp`

### 启动脚本与配置

- `start_linux.sh`
- `release/scripts/start_windows.bat`
- `release/config/app.env.example`
- `sql/permissions.sql`

### 文档

- `README.md`
- `release/README.md`
- `docs/项目使用说明.md`

## 四、测试与验证建议

建议合并前至少完成以下验证：

1. Linux / WSL 环境：
   - 使用 `./start_linux.sh run --db-pass='xxx'` 验证启动
   - 使用 `./start_linux.sh start --db-pass='xxx'` 验证后台启动
   - 验证 `PORT` 传参后后端监听端口正确变化
2. Windows 发布包：
   - 使用包含特殊字符的 `DB_PASS` 验证 `release/scripts/start_windows.bat` 读取配置是否正常
3. 前端入口：
   - 有 `vue-frontend/dist` 时验证根路径是否正确进入 Vue 页面
   - 无 Vue 产物时验证是否可回退到旧版页面
4. 接口异常：
   - 人工制造数据库异常，验证前端不再看到内部异常细节

## 五、风险与后续建议

本次修改已完成启动链路、配置安全和错误回显方面的第一轮修复，但仍建议后续继续推进：

1. 密码哈希从当前方案升级到更安全的实现（如 bcrypt / Argon2）。
2. 会话管理增加过期、持久化或签名机制。
3. 演示账号和测试账号口令与真实部署配置进一步解耦。
4. 逐步把敏感 SQL 路径改为预处理语句封装。

## 六、提交建议

建议使用以下中文 Commit 信息：

```text
修复启动链路并加固跨平台配置与错误处理
```
