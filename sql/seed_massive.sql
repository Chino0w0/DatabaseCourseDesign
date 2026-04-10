-- ============================================================
-- 社区健康档案管理系统 — 大批量测试样例数据（200+）
-- 适用: MySQL 8.0+
-- 用途: 压测 / 联调 / 分页与统计测试
-- 说明:
--   1) 脚本尽量幂等（大量使用 INSERT IGNORE / NOT EXISTS）
--   2) 需先执行 init.sql（确保基础表、角色、疾病字典存在）
-- ============================================================

USE community_health;

-- 强制连接使用 UTF-8，避免中文乱码
SET NAMES utf8mb4;
SET character_set_client = utf8mb4;
SET character_set_connection = utf8mb4;
SET character_set_results = utf8mb4;

START TRANSACTION;

-- ============================================================
-- 0. 准备测试医护账号（用于 measured_by / visitor_user_id）
-- ============================================================
-- 密码均为 123456 的 SHA-256 简化哈希
INSERT IGNORE INTO `users` (`username`, `password_hash`, `real_name`, `phone`)
VALUES
('doctor_perf_1', '8d969eef6ecad3c29a3a629280e686cf0c3f5d5a86aff3ca12020c923adc6c92', '压测医生甲', '13890000001'),
('doctor_perf_2', '8d969eef6ecad3c29a3a629280e686cf0c3f5d5a86aff3ca12020c923adc6c92', '压测医生乙', '13890000002'),
('nurse_perf_1',  '8d969eef6ecad3c29a3a629280e686cf0c3f5d5a86aff3ca12020c923adc6c92', '压测护士甲', '13890000003');

INSERT IGNORE INTO `user_roles` (`user_id`, `role_id`)
SELECT u.id, r.id
FROM `users` u
JOIN `roles` r ON r.role_name = '医生'
WHERE u.username IN ('doctor_perf_1', 'doctor_perf_2');

INSERT IGNORE INTO `user_roles` (`user_id`, `role_id`)
SELECT u.id, r.id
FROM `users` u
JOIN `roles` r ON r.role_name = '护士'
WHERE u.username = 'nurse_perf_1';

SET @doctor1_id = (SELECT id FROM `users` WHERE username = 'doctor_perf_1' LIMIT 1);
SET @doctor2_id = (SELECT id FROM `users` WHERE username = 'doctor_perf_2' LIMIT 1);
SET @nurse1_id  = (SELECT id FROM `users` WHERE username = 'nurse_perf_1' LIMIT 1);

-- ============================================================
-- 1. 批量居民（200条）
-- 规则:
--   - name: 压测居民001..200
--   - id_card: 18位、可复现且唯一（990101 + 8位 + 4位）
-- ============================================================
INSERT IGNORE INTO `residents` (
  `name`, `gender`, `id_card`, `birth_date`, `phone`,
  `community_id`, `address`, `emergency_contact`, `emergency_phone`
)
WITH RECURSIVE seq AS (
  SELECT 1 AS n
  UNION ALL
  SELECT n + 1 FROM seq WHERE n < 200
)
SELECT
  CONCAT('压测居民', LPAD(n, 3, '0')) AS `name`,
  IF(MOD(n, 2) = 0, '女', '男') AS `gender`,
  CONCAT('990101', LPAD(60000000 + n, 8, '0'), LPAD(n, 4, '0')) AS `id_card`,
  DATE_ADD('1950-01-01', INTERVAL MOD(n * 137, 25000) DAY) AS `birth_date`,
  CONCAT('139', LPAD(70000000 + n, 8, '0')) AS `phone`,
  MOD(n - 1, 3) + 1 AS `community_id`,
  CONCAT('压测小区', MOD(n - 1, 12) + 1, '栋', MOD(n - 1, 6) + 1, '单元', MOD(n - 1, 18) + 101, '室') AS `address`,
  CONCAT('联系人', LPAD(n, 3, '0')) AS `emergency_contact`,
  CONCAT('137', LPAD(60000000 + n, 8, '0')) AS `emergency_phone`
FROM seq;

-- ============================================================
-- 2. 为每个压测居民创建可登录账户 + 居民角色 + 账号绑定
-- ============================================================
INSERT IGNORE INTO `users` (`username`, `password_hash`, `real_name`, `phone`)
SELECT
  CONCAT('resident_perf_', RIGHT(r.id_card, 6)) AS username,
  '8d969eef6ecad3c29a3a629280e686cf0c3f5d5a86aff3ca12020c923adc6c92' AS password_hash,
  CONCAT(r.name, '账号') AS real_name,
  r.phone
FROM `residents` r
WHERE r.id_card LIKE '990101%';

INSERT IGNORE INTO `user_roles` (`user_id`, `role_id`)
SELECT u.id, rr.id
FROM `users` u
JOIN `roles` rr ON rr.role_name = '居民'
JOIN `residents` r ON u.username = CONCAT('resident_perf_', RIGHT(r.id_card, 6))
WHERE r.id_card LIKE '990101%';

INSERT IGNORE INTO `resident_accounts` (`user_id`, `resident_id`)
SELECT u.id, r.id
FROM `residents` r
JOIN `users` u ON u.username = CONCAT('resident_perf_', RIGHT(r.id_card, 6))
WHERE r.id_card LIKE '990101%';

-- ============================================================
-- 3. 补充健康档案摘要（health_records 由触发器自动建档）
-- ============================================================
UPDATE `health_records` hr
JOIN `residents` r ON r.id = hr.resident_id
SET
  hr.blood_type = CASE MOD(r.id, 4)
    WHEN 0 THEN 'A型'
    WHEN 1 THEN 'B型'
    WHEN 2 THEN 'O型'
    ELSE 'AB型'
  END,
  hr.allergy_history = CASE WHEN MOD(r.id, 7) = 0 THEN '青霉素过敏' ELSE '无' END,
  hr.family_history = CASE WHEN MOD(r.id, 5) = 0 THEN '父母有高血压史' ELSE '无明显家族史' END,
  hr.past_medical_history = CASE WHEN MOD(r.id, 9) = 0 THEN '既往有住院治疗史' ELSE '无' END
WHERE r.id_card LIKE '990101%';

-- ============================================================
-- 4. 健康测量数据（每位压测居民3条，共约600条）
-- ============================================================
INSERT INTO `health_measurements` (
  `resident_id`, `systolic`, `diastolic`, `blood_sugar`, `heart_rate`,
  `height`, `weight`, `bmi`, `notes`, `measured_by`, `measured_at`
)
WITH RECURSIVE m AS (
  SELECT 1 AS k
  UNION ALL
  SELECT k + 1 FROM m WHERE k < 3
)
SELECT
  r.id AS resident_id,
  CASE
    WHEN MOD(r.id, 10) = 0 THEN 155 + m.k
    WHEN MOD(r.id, 8) = 0 THEN 90 + m.k
    ELSE 118 + MOD(r.id + m.k, 18)
  END AS systolic,
  CASE
    WHEN MOD(r.id, 10) = 0 THEN 96 + m.k
    WHEN MOD(r.id, 8) = 0 THEN 58 + m.k
    ELSE 76 + MOD(r.id + m.k, 12)
  END AS diastolic,
  CASE
    WHEN MOD(r.id, 11) = 0 THEN ROUND(7.1 + m.k * 0.2, 2)
    ELSE ROUND(4.8 + MOD(r.id, 18) * 0.1 + m.k * 0.05, 2)
  END AS blood_sugar,
  62 + MOD(r.id + m.k, 35) AS heart_rate,
  ROUND(155 + MOD(r.id, 25) + MOD(m.k, 2) * 0.5, 1) AS height,
  ROUND(50 + MOD(r.id, 35) + m.k * 0.4, 1) AS weight,
  ROUND(
    (50 + MOD(r.id, 35) + m.k * 0.4) /
    POW((155 + MOD(r.id, 25) + MOD(m.k, 2) * 0.5) / 100, 2),
    1
  ) AS bmi,
  CASE
    WHEN MOD(r.id, 10) = 0 THEN '血压偏高，建议复测并门诊随访'
    WHEN MOD(r.id, 11) = 0 THEN '血糖偏高，建议空腹复查'
    WHEN MOD(r.id, 8) = 0 THEN '血压偏低，建议加强营养'
    ELSE '指标基本平稳'
  END AS notes,
  CASE
    WHEN m.k = 1 THEN @doctor1_id
    WHEN m.k = 2 THEN @doctor2_id
    ELSE @nurse1_id
  END AS measured_by,
  DATE_ADD('2026-01-01 08:00:00', INTERVAL (MOD(r.id, 200) * 3 + m.k) DAY) AS measured_at
FROM `residents` r
JOIN m
WHERE r.id_card LIKE '990101%'
  AND NOT EXISTS (
    SELECT 1
    FROM `health_measurements` hm
    WHERE hm.resident_id = r.id
      AND hm.measured_at = DATE_ADD('2026-01-01 08:00:00', INTERVAL (MOD(r.id, 200) * 3 + m.k) DAY)
  );

-- ============================================================
-- 5. 慢性病关联（按规则分配，约200+条）
-- ============================================================
INSERT IGNORE INTO `resident_diseases` (
  `resident_id`, `disease_id`, `diagnosed_date`, `status`, `notes`
)
SELECT r.id, 1, DATE_SUB(CURDATE(), INTERVAL (MOD(r.id, 1200) + 400) DAY), '控制中', '压测生成：高血压长期管理'
FROM `residents` r
WHERE r.id_card LIKE '990101%'
  AND MOD(r.id, 3) = 0
UNION ALL
SELECT r.id, 2, DATE_SUB(CURDATE(), INTERVAL (MOD(r.id, 1000) + 200) DAY), '治疗中', '压测生成：2型糖尿病随访中'
FROM `residents` r
WHERE r.id_card LIKE '990101%'
  AND MOD(r.id, 5) = 0
UNION ALL
SELECT r.id, 3, DATE_SUB(CURDATE(), INTERVAL (MOD(r.id, 900) + 150) DAY), '治疗中', '压测生成：冠心病管理中'
FROM `residents` r
WHERE r.id_card LIKE '990101%'
  AND MOD(r.id, 9) = 0;

-- ============================================================
-- 6. 随访记录（每位压测居民2条，共约400条）
-- ============================================================
INSERT INTO `visit_logs` (
  `resident_id`, `visitor_user_id`, `visit_type`, `visit_date`, `content`, `next_visit_date`
)
WITH RECURSIVE v AS (
  SELECT 1 AS k
  UNION ALL
  SELECT k + 1 FROM v WHERE k < 2
)
SELECT
  r.id,
  CASE WHEN v.k = 1 THEN @doctor1_id ELSE @doctor2_id END AS visitor_user_id,
  CASE WHEN MOD(r.id + v.k, 3) = 0 THEN '上门随访'
       WHEN MOD(r.id + v.k, 3) = 1 THEN '电话随访'
       ELSE '门诊随访' END AS visit_type,
  DATE_ADD('2026-02-01', INTERVAL (MOD(r.id, 200) * 2 + v.k) DAY) AS visit_date,
  CONCAT('压测随访#', v.k, '：居民', r.name, '，已完成健康宣教与风险评估。') AS content,
  DATE_ADD('2026-02-01', INTERVAL (MOD(r.id, 200) * 2 + v.k + 30) DAY) AS next_visit_date
FROM `residents` r
JOIN v
WHERE r.id_card LIKE '990101%'
  AND NOT EXISTS (
    SELECT 1
    FROM `visit_logs` vl
    WHERE vl.resident_id = r.id
      AND vl.visit_date = DATE_ADD('2026-02-01', INTERVAL (MOD(r.id, 200) * 2 + v.k) DAY)
  );

COMMIT;

-- ============================================================
-- 7. 导入结果检查
-- ============================================================
SELECT '压测居民数量' AS metric, COUNT(*) AS total
FROM `residents`
WHERE `id_card` LIKE '990101%'
UNION ALL
SELECT '压测居民登录账号数量', COUNT(*)
FROM `users` u
WHERE u.username LIKE 'resident_perf_%'
UNION ALL
SELECT '压测居民账号绑定数量', COUNT(*)
FROM `resident_accounts` ra
JOIN `residents` r ON r.id = ra.resident_id
WHERE r.id_card LIKE '990101%'
UNION ALL
SELECT '压测居民测量数据量', COUNT(*)
FROM `health_measurements` hm
JOIN `residents` r ON r.id = hm.resident_id
WHERE r.id_card LIKE '990101%'
UNION ALL
SELECT '压测居民随访记录量', COUNT(*)
FROM `visit_logs` vl
JOIN `residents` r ON r.id = vl.resident_id
WHERE r.id_card LIKE '990101%'
UNION ALL
SELECT '压测居民慢病关联量', COUNT(*)
FROM `resident_diseases` rd
JOIN `residents` r ON r.id = rd.resident_id
WHERE r.id_card LIKE '990101%';
