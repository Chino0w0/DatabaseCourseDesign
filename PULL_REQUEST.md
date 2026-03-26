# 📋 新增项目架构文档与数据库设计

## 概述

为社区健康档案管理系统建立基础文档体系和完整的数据库设计。

本 PR 定义了项目架构、模块划分、RESTful API 接口规范，以及满足第三范式 (3NF) 的 MySQL 数据库结构（含视图、存储过程、触发器和权限控制）。为后续全部开发工作奠定基础。

## 变更内容

### 文档 (`docs/`)

| 文件 | 说明 |
|------|------|
| `项目分工说明.md` | 按 8 个程序模块划分的开发分工，含模块依赖关系图和 Git 分支策略 |
| `课程设计要求.md` | 课程设计考核要求及非程序任务（绘图、报告大纲、PPT 结构） |
| `API接口文档.md` | RESTful API 规范 — 7 大模块、20+ 个接口，含请求/响应示例 |
| `数据库设计文档.md` | 10 张核心表 + 2 张辅助表、ER 图、索引设计、3NF 分析、视图/存储过程/触发器/权限概览 |

### SQL 脚本 (`sql/`)

| 文件 | 说明 |
|------|------|
| `init.sql` | 建库建表 DDL (10 张表) + 初始角色、管理员、社区、慢性病字典数据 |
| `views.sql` | 5 个查询视图（居民信息、最新测量含预警、慢性病详情、随访详情、系统概览） |
| `procedures.sql` | 3 个存储过程（测量录入自动计算 BMI 及预警、健康档案综合查询、社区健康统计） |
| `triggers.sql` | 4 个触发器（居民新增自动建档、删除/修改审计日志、测量异常自动预警）+ `audit_logs` 和 `health_warnings` 辅助表 |
| `permissions.sql` | 4 级用户权限模型（管理员 / 医生 / 护士 / 只读） |
| `seed.sql` | 测试种子数据：6 名居民、健康测量记录、慢性病关联、随访记录 |

### 其他

- `README.md` — 更新项目概览、目录结构和快速开始指南

## 数据库表关系

```
users ──┬── user_roles ──── roles
        ├── health_measurements
        └── visit_logs

communities ──── residents ──┬── health_records
                             ├── health_measurements → health_warnings (触发器)
                             ├── resident_diseases ──── chronic_diseases
                             ├── visit_logs
                             └── audit_logs (触发器)
```

## 系统架构

```
前端 (HTML/JS)  ←─ JSON/REST ─→  C++ 后端 (cpp-httplib)  ←─ SQL ─→  MySQL
                                  ├── models/      (实体层)
                                  ├── dao/         (数据访问层)
                                  ├── services/    (业务逻辑层)
                                  └── controllers/ (路由接口层)
```

## 验证方式

```bash
# 按顺序执行以初始化数据库
mysql -u root -p < sql/init.sql
mysql -u root -p community_health < sql/views.sql
mysql -u root -p community_health < sql/procedures.sql
mysql -u root -p community_health < sql/triggers.sql
mysql -u root -p community_health < sql/permissions.sql
mysql -u root -p community_health < sql/seed.sql
```

## 后续计划

- [ ] 实现 C++ 后端基础架构（模块 1）
- [ ] 实现用户认证模块（模块 2）
- [ ] 实现居民档案与健康模块（模块 3-6）
- [ ] 构建前端页面（模块 8）
