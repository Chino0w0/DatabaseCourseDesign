-- ============================================================
-- 社区健康档案管理系统 — 视图 (Views)
-- 课程设计要求：根据系统功能需求设计相应的查询视图
-- ============================================================

USE community_health;

-- ============================================================
-- 视图 1：居民完整信息视图
-- 说明：关联居民表与社区表，方便查询居民及其所属社区信息
-- ============================================================
CREATE OR REPLACE VIEW `v_resident_info` AS
SELECT
    r.id              AS resident_id,
    r.name            AS resident_name,
    r.gender,
    r.id_card,
    r.birth_date,
    TIMESTAMPDIFF(YEAR, r.birth_date, CURDATE()) AS age,
    r.phone,
    c.name            AS community_name,
    c.address          AS community_address,
    r.address          AS home_address,
    r.emergency_contact,
    r.emergency_phone,
    r.created_at       AS register_date
FROM `residents` r
LEFT JOIN `communities` c ON r.community_id = c.id;

-- ============================================================
-- 视图 2：居民最新健康测量视图
-- 说明：每位居民仅显示最近一次测量数据，用于仪表盘概览
-- ============================================================
CREATE OR REPLACE VIEW `v_latest_measurement` AS
SELECT
    hm.id              AS measurement_id,
    hm.resident_id,
    r.name             AS resident_name,
    r.gender,
    hm.systolic,
    hm.diastolic,
    hm.blood_sugar,
    hm.heart_rate,
    hm.height,
    hm.weight,
    hm.bmi,
    hm.measured_at,
    u.real_name        AS measured_by_name,
    -- 血压预警标记
    CASE
        WHEN hm.systolic >= 140 OR hm.diastolic >= 90 THEN '高血压预警'
        WHEN hm.systolic <= 90 OR hm.diastolic <= 60 THEN '低血压预警'
        ELSE '正常'
    END AS bp_warning,
    -- 血糖预警标记
    CASE
        WHEN hm.blood_sugar >= 7.0 THEN '血糖偏高'
        WHEN hm.blood_sugar IS NOT NULL AND hm.blood_sugar <= 3.9 THEN '血糖偏低'
        ELSE '正常'
    END AS sugar_warning
FROM `health_measurements` hm
INNER JOIN (
    SELECT resident_id, MAX(measured_at) AS latest_time
    FROM `health_measurements`
    GROUP BY resident_id
) latest ON hm.resident_id = latest.resident_id AND hm.measured_at = latest.latest_time
LEFT JOIN `residents` r ON hm.resident_id = r.id
LEFT JOIN `users` u ON hm.measured_by = u.id;

-- ============================================================
-- 视图 3：居民慢性病详情视图
-- 说明：关联居民表、慢性病字典表和关联表，展示居民患病情况
-- ============================================================
CREATE OR REPLACE VIEW `v_resident_disease_detail` AS
SELECT
    r.id               AS resident_id,
    r.name             AS resident_name,
    r.gender,
    cd.disease_name,
    cd.disease_code,
    cd.category        AS disease_category,
    rd.diagnosed_date,
    rd.status          AS disease_status,
    rd.notes
FROM `resident_diseases` rd
INNER JOIN `residents` r ON rd.resident_id = r.id
INNER JOIN `chronic_diseases` cd ON rd.disease_id = cd.id;

-- ============================================================
-- 视图 4：随访记录详情视图
-- 说明：关联随访表、居民表和用户表，展示完整随访信息
-- ============================================================
CREATE OR REPLACE VIEW `v_visit_detail` AS
SELECT
    vl.id              AS visit_id,
    vl.resident_id,
    r.name             AS resident_name,
    r.phone            AS resident_phone,
    c.name             AS community_name,
    vl.visitor_user_id,
    u.real_name        AS visitor_name,
    vl.visit_type,
    vl.visit_date,
    vl.content,
    vl.next_visit_date,
    vl.created_at
FROM `visit_logs` vl
INNER JOIN `residents` r ON vl.resident_id = r.id
LEFT JOIN `communities` c ON r.community_id = c.id
INNER JOIN `users` u ON vl.visitor_user_id = u.id;

-- ============================================================
-- 视图 5：系统统计概览视图（用于仪表盘）
-- 说明：汇总各个维度的统计数量
-- ============================================================
CREATE OR REPLACE VIEW `v_system_overview` AS
SELECT
    (SELECT COUNT(*) FROM `residents`)             AS total_residents,
    (SELECT COUNT(*) FROM `health_measurements`)   AS total_measurements,
    (SELECT COUNT(*) FROM `visit_logs`)            AS total_visits,
    (SELECT COUNT(*) FROM `communities`)           AS total_communities,
    (SELECT COUNT(*) FROM `resident_diseases`)     AS total_disease_records,
    (SELECT COUNT(DISTINCT resident_id) FROM `health_measurements`
     WHERE systolic >= 140 OR diastolic >= 90)     AS bp_warning_count,
    (SELECT COUNT(DISTINCT resident_id) FROM `health_measurements`
     WHERE blood_sugar >= 7.0)                     AS sugar_warning_count;
