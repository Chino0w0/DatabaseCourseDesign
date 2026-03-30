-- ============================================================
-- 社区健康档案管理系统：需求覆盖测试数据（100-200 实例）
-- 文件名：seed_requirement_test.sql
-- 适用数据库：MySQL 8.0+
-- 字符集：utf8mb4
--
-- 建议导入顺序：
--   1) sql/init.sql
--   2) sql/views.sql
--   3) sql/procedures.sql
--   4) sql/triggers.sql
--   5) sql/seed_requirement_test.sql  <-- 当前文件
--
-- 说明：
--   - 本文件用于“主要数据结构”联调与课程设计演示。
--   - 覆盖：用户/角色、居民档案、健康测量、慢病关联、随访、
--           就诊记录、用药记录、疫苗接种、操作日志。
--   - 通过“测试前缀”保证可重复导入（先清理再插入）。
-- ============================================================

USE community_health;
SET NAMES utf8mb4;

START TRANSACTION;

-- ============================================================
-- 0) 扩展表（用于补齐需求中的就诊/用药/接种/日志）
-- ============================================================
CREATE TABLE IF NOT EXISTS `clinic_visits` (
  `id`            INT AUTO_INCREMENT PRIMARY KEY,
  `resident_id`   INT NOT NULL COMMENT '居民ID',
  `visit_time`    DATETIME NOT NULL COMMENT '就诊时间',
  `institution`   VARCHAR(120) NOT NULL COMMENT '就诊机构',
  `diagnosis`     VARCHAR(255) NOT NULL COMMENT '诊断结果',
  `prescription`  TEXT DEFAULT NULL COMMENT '处方',
  `report_summary`TEXT DEFAULT NULL COMMENT '检查报告摘要',
  `source_tag`    VARCHAR(40) DEFAULT 'seed_requirement_test',
  `created_at`    DATETIME DEFAULT CURRENT_TIMESTAMP,
  INDEX `idx_cv_resident_time` (`resident_id`,`visit_time`),
  CONSTRAINT `fk_cv_resident` FOREIGN KEY (`resident_id`) REFERENCES `residents`(`id`) ON DELETE CASCADE
) ENGINE=InnoDB COMMENT='就诊记录';

CREATE TABLE IF NOT EXISTS `medication_records` (
  `id`              INT AUTO_INCREMENT PRIMARY KEY,
  `resident_id`     INT NOT NULL COMMENT '居民ID',
  `drug_name`       VARCHAR(120) NOT NULL COMMENT '药品名称',
  `dose`            VARCHAR(60)  NOT NULL COMMENT '剂量',
  `usage_method`    VARCHAR(120) NOT NULL COMMENT '用法',
  `reason`          VARCHAR(255) DEFAULT NULL COMMENT '用药原因',
  `start_date`      DATE NOT NULL,
  `end_date`        DATE DEFAULT NULL,
  `prescribed_by`   INT DEFAULT NULL COMMENT '开药医生ID',
  `source_tag`      VARCHAR(40) DEFAULT 'seed_requirement_test',
  `created_at`      DATETIME DEFAULT CURRENT_TIMESTAMP,
  INDEX `idx_mr_resident_start` (`resident_id`,`start_date`),
  CONSTRAINT `fk_mr_resident` FOREIGN KEY (`resident_id`) REFERENCES `residents`(`id`) ON DELETE CASCADE,
  CONSTRAINT `fk_mr_user` FOREIGN KEY (`prescribed_by`) REFERENCES `users`(`id`) ON DELETE SET NULL
) ENGINE=InnoDB COMMENT='用药记录';

CREATE TABLE IF NOT EXISTS `vaccination_records` (
  `id`               INT AUTO_INCREMENT PRIMARY KEY,
  `resident_id`      INT NOT NULL COMMENT '居民ID',
  `vaccine_name`     VARCHAR(120) NOT NULL COMMENT '疫苗名称',
  `vaccination_date` DATE NOT NULL COMMENT '接种日期',
  `institution`      VARCHAR(120) NOT NULL COMMENT '接种机构',
  `doctor_name`      VARCHAR(60)  DEFAULT NULL COMMENT '接种医生',
  `source_tag`       VARCHAR(40) DEFAULT 'seed_requirement_test',
  `created_at`       DATETIME DEFAULT CURRENT_TIMESTAMP,
  INDEX `idx_vr_resident_date` (`resident_id`,`vaccination_date`),
  CONSTRAINT `fk_vr_resident` FOREIGN KEY (`resident_id`) REFERENCES `residents`(`id`) ON DELETE CASCADE
) ENGINE=InnoDB COMMENT='疫苗接种记录';

CREATE TABLE IF NOT EXISTS `operation_logs` (
  `id`            INT AUTO_INCREMENT PRIMARY KEY,
  `user_id`       INT NOT NULL,
  `module`        VARCHAR(80) NOT NULL,
  `action`        VARCHAR(120) NOT NULL,
  `detail`        VARCHAR(255) DEFAULT NULL,
  `operated_at`   DATETIME DEFAULT CURRENT_TIMESTAMP,
  `source_tag`    VARCHAR(40) DEFAULT 'seed_requirement_test',
  INDEX `idx_ol_user_time` (`user_id`,`operated_at`),
  CONSTRAINT `fk_ol_user` FOREIGN KEY (`user_id`) REFERENCES `users`(`id`) ON DELETE CASCADE
) ENGINE=InnoDB COMMENT='系统操作日志';

-- ============================================================
-- 1) 清理旧测试数据（支持重复导入）
-- ============================================================
DELETE FROM `operation_logs`       WHERE source_tag = 'seed_requirement_test';
DELETE FROM `vaccination_records`  WHERE source_tag = 'seed_requirement_test';
DELETE FROM `medication_records`   WHERE source_tag = 'seed_requirement_test';
DELETE FROM `clinic_visits`        WHERE source_tag = 'seed_requirement_test';

DELETE FROM `visit_logs`            WHERE content LIKE '测试随访#%';
DELETE FROM `health_measurements`   WHERE notes LIKE '测试测量#%';
DELETE FROM `resident_diseases`     WHERE notes LIKE '测试病种#%';

DELETE FROM `residents` WHERE id_card LIKE '51010999%';
DELETE FROM `users`     WHERE username LIKE 'test_%';

-- ============================================================
-- 2) 插入测试用户（8）与角色映射（8）
-- ============================================================
INSERT INTO `users` (`username`,`password_hash`,`real_name`,`phone`,`status`) VALUES
  ('test_admin_01',    '8d969eef6ecad3c29a3a629280e686cf0c3f5d5a86aff3ca12020c923adc6c92', '测试管理员A',   '13700000001', 1),
  ('test_doctor_01',   '8d969eef6ecad3c29a3a629280e686cf0c3f5d5a86aff3ca12020c923adc6c92', '测试医生张',    '13700000002', 1),
  ('test_doctor_02',   '8d969eef6ecad3c29a3a629280e686cf0c3f5d5a86aff3ca12020c923adc6c92', '测试医生李',    '13700000003', 1),
  ('test_doctor_03',   '8d969eef6ecad3c29a3a629280e686cf0c3f5d5a86aff3ca12020c923adc6c92', '测试医生王',    '13700000004', 1),
  ('test_nurse_01',    '8d969eef6ecad3c29a3a629280e686cf0c3f5d5a86aff3ca12020c923adc6c92', '测试护士赵',    '13700000005', 1),
  ('test_nurse_02',    '8d969eef6ecad3c29a3a629280e686cf0c3f5d5a86aff3ca12020c923adc6c92', '测试护士钱',    '13700000006', 1),
  ('test_resident_01', '8d969eef6ecad3c29a3a629280e686cf0c3f5d5a86aff3ca12020c923adc6c92', '测试居民账号甲', '13700000007', 1),
  ('test_resident_02', '8d969eef6ecad3c29a3a629280e686cf0c3f5d5a86aff3ca12020c923adc6c92', '测试居民账号乙', '13700000008', 1);

INSERT IGNORE INTO `user_roles` (`user_id`,`role_id`)
SELECT u.id, r.id
FROM `users` u
JOIN `roles` r
  ON (
    (u.username = 'test_admin_01' AND r.role_name = '管理员') OR
    (u.username LIKE 'test_doctor_%' AND r.role_name = '医生') OR
    (u.username LIKE 'test_nurse_%' AND r.role_name = '护士') OR
    (u.username LIKE 'test_resident_%' AND r.role_name = '护士')
  )
WHERE u.username LIKE 'test_%';

-- ============================================================
-- 3) 插入测试居民（36）
-- ============================================================
INSERT INTO `residents`
(`name`,`gender`,`id_card`,`birth_date`,`phone`,`community_id`,`address`,`emergency_contact`,`emergency_phone`)
WITH RECURSIVE seq AS (
  SELECT 1 AS n
  UNION ALL
  SELECT n + 1 FROM seq WHERE n < 36
)
SELECT
  CONCAT('测试居民', LPAD(n, 2, '0')) AS name,
  IF(MOD(n,2)=0,'女','男') AS gender,
  CONCAT('51010999', LPAD(n, 10, '0')) AS id_card,
  DATE_ADD('1960-01-01', INTERVAL n * 120 DAY) AS birth_date,
  CONCAT('139', LPAD(n, 8, '0')) AS phone,
  MOD(n-1,3) + 1 AS community_id,
  CONCAT('测试街道', MOD(n-1,3)+1, '号-', n, '室') AS address,
  CONCAT('家属', LPAD(n,2,'0')) AS emergency_contact,
  CONCAT('138', LPAD(80000000 + n, 8, '0')) AS emergency_phone
FROM seq;

-- 若触发器未导入，补齐一人一档
INSERT IGNORE INTO `health_records` (`resident_id`)
SELECT r.id
FROM `residents` r
WHERE r.id_card LIKE '51010999%';

-- 补充档案摘要信息（不新增行，仅更新）
UPDATE `health_records` hr
JOIN `residents` r ON r.id = hr.resident_id
SET
  hr.blood_type = ELT(MOD(r.id,4)+1, 'A型','B型','O型','AB型'),
  hr.allergy_history = CASE MOD(r.id,5)
    WHEN 0 THEN '无'
    WHEN 1 THEN '青霉素过敏'
    WHEN 2 THEN '花粉过敏'
    WHEN 3 THEN '海鲜过敏'
    ELSE '尘螨过敏'
  END,
  hr.family_history = CASE WHEN MOD(r.id,3)=0 THEN '父亲高血压' WHEN MOD(r.id,3)=1 THEN '母亲糖尿病' ELSE '无明显家族史' END,
  hr.past_medical_history = CASE WHEN MOD(r.id,4)=0 THEN '既往有轻度脂肪肝' ELSE '无重大手术史' END
WHERE r.id_card LIKE '51010999%';

-- ============================================================
-- 4) 健康测量（36）
-- ============================================================
INSERT INTO `health_measurements`
(`resident_id`,`systolic`,`diastolic`,`blood_sugar`,`heart_rate`,`height`,`weight`,`bmi`,`notes`,`measured_by`,`measured_at`)
SELECT
  r.id,
  110 + MOD(r.id * 3, 55) AS systolic,
  68 + MOD(r.id * 2, 28)  AS diastolic,
  ROUND(4.2 + MOD(r.id, 45) / 10, 2) AS blood_sugar,
  58 + MOD(r.id * 5, 50)  AS heart_rate,
  ROUND(155 + MOD(r.id, 25), 1) AS height,
  ROUND(50 + MOD(r.id * 2, 35), 1) AS weight,
  ROUND((50 + MOD(r.id * 2, 35)) / POW((155 + MOD(r.id, 25))/100, 2), 1) AS bmi,
  CONCAT('测试测量#', ROW_NUMBER() OVER (ORDER BY r.id)) AS notes,
  (SELECT MIN(id) FROM `users` WHERE username LIKE 'test_doctor_%') AS measured_by,
  DATE_SUB(NOW(), INTERVAL MOD(r.id, 30) DAY) AS measured_at
FROM `residents` r
WHERE r.id_card LIKE '51010999%'
ORDER BY r.id;

-- ============================================================
-- 5) 慢病关联（14）
-- ============================================================
INSERT IGNORE INTO `resident_diseases`
(`resident_id`,`disease_id`,`diagnosed_date`,`status`,`notes`)
SELECT
  t.resident_id,
  MOD(t.rn - 1, 5) + 1 AS disease_id,
  DATE_SUB(CURDATE(), INTERVAL (t.rn * 40) DAY) AS diagnosed_date,
  ELT(MOD(t.rn,3)+1, '治疗中','控制中','已痊愈') AS status,
  CONCAT('测试病种#', t.rn) AS notes
FROM (
  SELECT r.id AS resident_id, ROW_NUMBER() OVER (ORDER BY r.id) AS rn
  FROM `residents` r
  WHERE r.id_card LIKE '51010999%'
) t
WHERE t.rn <= 14;

-- ============================================================
-- 6) 随访记录（18）
-- ============================================================
INSERT INTO `visit_logs`
(`resident_id`,`visitor_user_id`,`visit_type`,`visit_date`,`content`,`next_visit_date`)
SELECT
  t.resident_id,
  (SELECT MIN(id) FROM `users` WHERE username LIKE 'test_doctor_%') AS visitor_user_id,
  ELT(MOD(t.rn,3)+1, '上门随访', '电话随访', '门诊随访') AS visit_type,
  DATE_SUB(CURDATE(), INTERVAL t.rn DAY) AS visit_date,
  CONCAT('测试随访#', t.rn, '：记录症状体征，给出饮食与运动建议。') AS content,
  DATE_ADD(CURDATE(), INTERVAL (7 + MOD(t.rn, 21)) DAY) AS next_visit_date
FROM (
  SELECT r.id AS resident_id, ROW_NUMBER() OVER (ORDER BY r.id) AS rn
  FROM `residents` r
  WHERE r.id_card LIKE '51010999%'
) t
WHERE t.rn <= 18;

-- ============================================================
-- 7) 就诊记录（10）
-- ============================================================
INSERT INTO `clinic_visits`
(`resident_id`,`visit_time`,`institution`,`diagnosis`,`prescription`,`report_summary`,`source_tag`)
SELECT
  t.resident_id,
  DATE_SUB(NOW(), INTERVAL (t.rn * 3) DAY) AS visit_time,
  ELT(MOD(t.rn,3)+1, '社区卫生服务中心', '区人民医院', '市中医院') AS institution,
  ELT(MOD(t.rn,4)+1, '原发性高血压', '2型糖尿病', '上呼吸道感染', '血脂异常') AS diagnosis,
  ELT(MOD(t.rn,4)+1, '氨氯地平片', '二甲双胍片', '复方氨酚烷胺', '阿托伐他汀') AS prescription,
  CONCAT('检查摘要#', t.rn, '：生命体征稳定，建议复诊。') AS report_summary,
  'seed_requirement_test'
FROM (
  SELECT r.id AS resident_id, ROW_NUMBER() OVER (ORDER BY r.id) AS rn
  FROM `residents` r
  WHERE r.id_card LIKE '51010999%'
) t
WHERE t.rn <= 10;

-- ============================================================
-- 8) 用药记录（16）
-- ============================================================
INSERT INTO `medication_records`
(`resident_id`,`drug_name`,`dose`,`usage_method`,`reason`,`start_date`,`end_date`,`prescribed_by`,`source_tag`)
SELECT
  t.resident_id,
  ELT(MOD(t.rn,4)+1, '氨氯地平片', '二甲双胍片', '阿司匹林肠溶片', '辛伐他汀片') AS drug_name,
  ELT(MOD(t.rn,4)+1, '5mg', '0.5g', '100mg', '20mg') AS dose,
  ELT(MOD(t.rn,3)+1, '每日1次，晨服', '每日2次，早晚饭后', '每日1次，睡前') AS usage_method,
  ELT(MOD(t.rn,4)+1, '控制血压', '控制血糖', '抗血小板', '调脂') AS reason,
  DATE_SUB(CURDATE(), INTERVAL (t.rn * 5) DAY) AS start_date,
  DATE_ADD(CURDATE(), INTERVAL (30 + MOD(t.rn,20)) DAY) AS end_date,
  (SELECT MIN(id) FROM `users` WHERE username LIKE 'test_doctor_%') AS prescribed_by,
  'seed_requirement_test'
FROM (
  SELECT r.id AS resident_id, ROW_NUMBER() OVER (ORDER BY r.id) AS rn
  FROM `residents` r
  WHERE r.id_card LIKE '51010999%'
) t
WHERE t.rn <= 16;

-- ============================================================
-- 9) 接种记录（10）
-- ============================================================
INSERT INTO `vaccination_records`
(`resident_id`,`vaccine_name`,`vaccination_date`,`institution`,`doctor_name`,`source_tag`)
SELECT
  t.resident_id,
  ELT(MOD(t.rn,4)+1, '流感疫苗', '23价肺炎疫苗', '乙肝疫苗', '新冠加强针') AS vaccine_name,
  DATE_SUB(CURDATE(), INTERVAL (t.rn * 18) DAY) AS vaccination_date,
  '社区预防接种门诊' AS institution,
  ELT(MOD(t.rn,3)+1, '张医生', '李医生', '王医生') AS doctor_name,
  'seed_requirement_test'
FROM (
  SELECT r.id AS resident_id, ROW_NUMBER() OVER (ORDER BY r.id) AS rn
  FROM `residents` r
  WHERE r.id_card LIKE '51010999%'
) t
WHERE t.rn <= 10;

-- ============================================================
-- 10) 操作日志（8）
-- ============================================================
INSERT INTO `operation_logs`
(`user_id`,`module`,`action`,`detail`,`operated_at`,`source_tag`)
SELECT
  u.id,
  ELT(MOD(ROW_NUMBER() OVER (ORDER BY u.id),4)+1, '居民档案', '健康测量', '随访管理', '系统管理') AS module,
  ELT(MOD(ROW_NUMBER() OVER (ORDER BY u.id),4)+1, '新增记录', '更新记录', '查询统计', '导出报表') AS action,
  CONCAT('测试操作日志#', ROW_NUMBER() OVER (ORDER BY u.id), ' by ', u.username) AS detail,
  DATE_SUB(NOW(), INTERVAL ROW_NUMBER() OVER (ORDER BY u.id) HOUR) AS operated_at,
  'seed_requirement_test'
FROM `users` u
WHERE u.username LIKE 'test_%'
ORDER BY u.id
LIMIT 8;

COMMIT;

-- ============================================================
-- 数据规模（理论新增行）：
--   users 8
--   user_roles 8
--   residents 36
--   health_records 36
--   health_measurements 36
--   resident_diseases 14
--   visit_logs 18
--   clinic_visits 10
--   medication_records 16
--   vaccination_records 10
--   operation_logs 8
-- 合计：200 行
-- ============================================================
