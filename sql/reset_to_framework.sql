-- ============================================================
-- 社区健康档案管理系统 — 框架级清空脚本
-- 目标：清空业务数据，仅保留基础框架（表结构/角色/慢病字典/管理员）
-- 适用：MySQL 8.0+
-- ============================================================

USE community_health;

SET NAMES utf8mb4;
SET character_set_client = utf8mb4;
SET character_set_connection = utf8mb4;
SET character_set_results = utf8mb4;

START TRANSACTION;

-- 1) 先删依赖数据（子表 -> 主表）
DELETE FROM visit_logs;
DELETE FROM health_measurements;
DELETE FROM resident_diseases;
DELETE FROM health_records;
DELETE FROM resident_accounts;

-- 2) 清空居民
DELETE FROM residents;

-- 3) 清空社区（含历史压测社区/脏社区）
DELETE FROM communities;

-- 4) 清理非管理员用户及其角色（保留 admin）
DELETE ur
FROM user_roles ur
JOIN users u ON u.id = ur.user_id
WHERE u.username <> 'admin';

DELETE FROM users
WHERE username <> 'admin';

COMMIT;

-- 5) 清空后检查（应仅剩框架数据）
SELECT 'users_total' AS metric, COUNT(*) AS total FROM users
UNION ALL
SELECT 'users_admin', COUNT(*) FROM users WHERE username = 'admin'
UNION ALL
SELECT 'roles_total', COUNT(*) FROM roles
UNION ALL
SELECT 'diseases_total', COUNT(*) FROM chronic_diseases
UNION ALL
SELECT 'communities_total', COUNT(*) FROM communities
UNION ALL
SELECT 'residents_total', COUNT(*) FROM residents
UNION ALL
SELECT 'health_measurements_total', COUNT(*) FROM health_measurements
UNION ALL
SELECT 'visit_logs_total', COUNT(*) FROM visit_logs;
