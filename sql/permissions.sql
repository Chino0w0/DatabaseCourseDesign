-- ============================================================
-- 社区健康档案管理系统 — 用户权限管理 (User Permissions)
-- 课程设计要求：通过建立用户和权限分配实现数据库一定的安全性
-- ============================================================

USE community_health;

-- ============================================================
-- 1. 创建数据库用户
-- ============================================================

-- 执行前请先把下面的示例密码改成你自己的密码，
-- 不要直接在共享环境中使用这些占位值。

-- 管理员用户：拥有全部权限
CREATE USER IF NOT EXISTS 'ch_admin' @'localhost' IDENTIFIED BY 'PLEASE_CHANGE_CH_ADMIN_PASSWORD';

-- 医生用户：可查询和录入健康数据
CREATE USER IF NOT EXISTS 'ch_doctor' @'localhost' IDENTIFIED BY 'PLEASE_CHANGE_CH_DOCTOR_PASSWORD';

-- 护士用户：仅可录入测量数据，不可删除
CREATE USER IF NOT EXISTS 'ch_nurse' @'localhost' IDENTIFIED BY 'PLEASE_CHANGE_CH_NURSE_PASSWORD';

-- 只读用户：仅用于数据查询和报表
CREATE USER IF NOT EXISTS 'ch_readonly' @'localhost' IDENTIFIED BY 'PLEASE_CHANGE_CH_READONLY_PASSWORD';

-- ============================================================
-- 2. 权限分配
-- ============================================================

-- ----- 管理员：全部权限 -----
GRANT ALL PRIVILEGES ON community_health.* TO 'ch_admin' @'localhost';

-- ----- 医生：增删改查居民、健康、随访数据 -----
GRANT
SELECT,
INSERT
,
UPDATE ON community_health.residents TO 'ch_doctor' @'localhost';

GRANT
SELECT,
INSERT
,
UPDATE ON community_health.health_records TO 'ch_doctor' @'localhost';

GRANT
SELECT,
INSERT
,
UPDATE ON community_health.health_measurements TO 'ch_doctor' @'localhost';

GRANT
SELECT,
INSERT
,
UPDATE ON community_health.visit_logs TO 'ch_doctor' @'localhost';

GRANT
SELECT,
INSERT
,
UPDATE ON community_health.resident_diseases TO 'ch_doctor' @'localhost';

GRANT
SELECT ON community_health.chronic_diseases TO 'ch_doctor' @'localhost';

GRANT
SELECT ON community_health.communities TO 'ch_doctor' @'localhost';

GRANT
SELECT ON community_health.health_warnings TO 'ch_doctor' @'localhost';

-- 医生可以查看视图
GRANT
SELECT ON community_health.v_resident_info TO 'ch_doctor' @'localhost';

GRANT
SELECT ON community_health.v_latest_measurement TO 'ch_doctor' @'localhost';

GRANT
SELECT ON community_health.v_resident_disease_detail TO 'ch_doctor' @'localhost';

GRANT
SELECT ON community_health.v_visit_detail TO 'ch_doctor' @'localhost';

GRANT
SELECT ON community_health.v_system_overview TO 'ch_doctor' @'localhost';

-- 医生可以执行存储过程
GRANT EXECUTE ON PROCEDURE community_health.sp_add_measurement
  TO 'ch_doctor'@'localhost';

GRANT
EXECUTE ON PROCEDURE community_health.sp_get_resident_health_profile TO 'ch_doctor' @'localhost';

GRANT
EXECUTE ON PROCEDURE community_health.sp_community_health_stats TO 'ch_doctor' @'localhost';

-- ----- 护士：仅录入测量数据，不可删除/修改居民信息 -----
GRANT
SELECT ON community_health.residents TO 'ch_nurse' @'localhost';

GRANT
SELECT,
INSERT
    ON community_health.health_measurements TO 'ch_nurse' @'localhost';

GRANT
SELECT ON community_health.communities TO 'ch_nurse' @'localhost';

GRANT
SELECT ON community_health.chronic_diseases TO 'ch_nurse' @'localhost';

GRANT
SELECT ON community_health.health_warnings TO 'ch_nurse' @'localhost';

GRANT
SELECT ON community_health.v_resident_info TO 'ch_nurse' @'localhost';

GRANT
SELECT ON community_health.v_latest_measurement TO 'ch_nurse' @'localhost';

GRANT
EXECUTE ON PROCEDURE community_health.sp_add_measurement TO 'ch_nurse' @'localhost';

-- ----- 只读用户：所有表和视图的 SELECT 权限 -----
GRANT SELECT ON community_health.* TO 'ch_readonly' @'localhost';

-- ============================================================
-- 3. 刷新权限
-- ============================================================
FLUSH PRIVILEGES;

-- ============================================================
-- 4. 权限验证说明
-- ============================================================
-- 验证方式（请把示例密码替换成你在本文件中设置的实际密码）：
--   mysql -u ch_doctor -p'<your_doctor_password>' community_health
--   > INSERT INTO health_measurements (...) VALUES (...);   -- 应成功
--   > DELETE FROM residents WHERE id = 1;                   -- 应失败 (无 DELETE 权限)
--
--   mysql -u ch_nurse -p'<your_nurse_password>' community_health
--   > INSERT INTO health_measurements (...) VALUES (...);   -- 应成功
--   > UPDATE residents SET name='test' WHERE id=1;          -- 应失败 (无 UPDATE 权限)
--
--   mysql -u ch_readonly -p'<your_readonly_password>' community_health
--   > SELECT * FROM v_system_overview;                      -- 应成功
--   > INSERT INTO residents (...) VALUES (...);             -- 应失败 (无 INSERT 权限)