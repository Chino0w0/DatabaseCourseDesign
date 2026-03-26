-- ============================================================
-- 社区健康档案管理系统 — 存储过程 (Stored Procedures)
-- 课程设计要求：根据系统功能需求建立存储过程
-- ============================================================

USE community_health;

DELIMITER //

-- ============================================================
-- 存储过程 1：录入健康测量数据并自动计算 BMI
-- 参数：居民ID、收缩压、舒张压、血糖、心率、身高、体重、测量人ID
-- 功能：自动计算 BMI = 体重(kg) / (身高(m))^2
-- 返回：插入的测量记录 ID 和预警信息
-- ============================================================
CREATE PROCEDURE `sp_add_measurement` (
    IN  p_resident_id  INT,
    IN  p_systolic     INT,
    IN  p_diastolic    INT,
    IN  p_blood_sugar  DECIMAL(5,2),
    IN  p_heart_rate   INT,
    IN  p_height       DECIMAL(5,1),
    IN  p_weight       DECIMAL(5,1),
    IN  p_measured_by  INT,
    IN  p_notes        TEXT,
    OUT p_measurement_id INT,
    OUT p_warning_msg    VARCHAR(500)
)
BEGIN
    DECLARE v_bmi DECIMAL(4,1) DEFAULT NULL;
    DECLARE v_warning VARCHAR(500) DEFAULT '';

    -- 计算 BMI（身高从 cm 转为 m）
    IF p_height IS NOT NULL AND p_weight IS NOT NULL AND p_height > 0 THEN
        SET v_bmi = ROUND(p_weight / POW(p_height / 100, 2), 1);
    END IF;

    -- 生成预警信息
    IF p_systolic >= 140 OR p_diastolic >= 90 THEN
        SET v_warning = CONCAT(v_warning, '【高血压预警】收缩压:', p_systolic, ' 舒张压:', p_diastolic, '; ');
    END IF;

    IF p_systolic IS NOT NULL AND (p_systolic <= 90 OR p_diastolic <= 60) THEN
        SET v_warning = CONCAT(v_warning, '【低血压预警】收缩压:', p_systolic, ' 舒张压:', p_diastolic, '; ');
    END IF;

    IF p_blood_sugar >= 7.0 THEN
        SET v_warning = CONCAT(v_warning, '【血糖偏高】血糖值:', p_blood_sugar, ' mmol/L; ');
    ELSEIF p_blood_sugar IS NOT NULL AND p_blood_sugar <= 3.9 THEN
        SET v_warning = CONCAT(v_warning, '【血糖偏低】血糖值:', p_blood_sugar, ' mmol/L; ');
    END IF;

    IF v_bmi IS NOT NULL AND v_bmi >= 28.0 THEN
        SET v_warning = CONCAT(v_warning, '【肥胖预警】BMI:', v_bmi, '; ');
    END IF;

    IF p_heart_rate IS NOT NULL AND (p_heart_rate > 100 OR p_heart_rate < 60) THEN
        SET v_warning = CONCAT(v_warning, '【心率异常】心率:', p_heart_rate, '次/分; ');
    END IF;

    -- 插入测量记录
    INSERT INTO `health_measurements` (
        resident_id, systolic, diastolic, blood_sugar,
        heart_rate, height, weight, bmi, notes, measured_by
    ) VALUES (
        p_resident_id, p_systolic, p_diastolic, p_blood_sugar,
        p_heart_rate, p_height, p_weight, v_bmi, p_notes, p_measured_by
    );

    SET p_measurement_id = LAST_INSERT_ID();
    SET p_warning_msg = IF(v_warning = '', '各项指标正常', v_warning);
END //

-- ============================================================
-- 存储过程 2：居民健康档案综合查询
-- 参数：居民ID
-- 功能：一次性返回居民基本信息、最近测量记录、慢性病、随访记录
-- ============================================================
CREATE PROCEDURE `sp_get_resident_health_profile` (
    IN p_resident_id INT
)
BEGIN
    -- 结果集 1：居民基本信息
    SELECT r.*, c.name AS community_name
    FROM `residents` r
    LEFT JOIN `communities` c ON r.community_id = c.id
    WHERE r.id = p_resident_id;

    -- 结果集 2：健康档案摘要
    SELECT *
    FROM `health_records`
    WHERE resident_id = p_resident_id;

    -- 结果集 3：最近 10 条测量记录
    SELECT hm.*, u.real_name AS measured_by_name
    FROM `health_measurements` hm
    LEFT JOIN `users` u ON hm.measured_by = u.id
    WHERE hm.resident_id = p_resident_id
    ORDER BY hm.measured_at DESC
    LIMIT 10;

    -- 结果集 4：慢性病列表
    SELECT cd.disease_name, cd.disease_code, rd.diagnosed_date, rd.status, rd.notes
    FROM `resident_diseases` rd
    INNER JOIN `chronic_diseases` cd ON rd.disease_id = cd.id
    WHERE rd.resident_id = p_resident_id;

    -- 结果集 5：最近 5 条随访记录
    SELECT vl.*, u.real_name AS visitor_name
    FROM `visit_logs` vl
    INNER JOIN `users` u ON vl.visitor_user_id = u.id
    WHERE vl.resident_id = p_resident_id
    ORDER BY vl.visit_date DESC
    LIMIT 5;
END //

-- ============================================================
-- 存储过程 3：按社区统计健康预警数据
-- 参数：无
-- 功能：统计每个社区的居民数、高血压人数、高血糖人数
-- ============================================================
CREATE PROCEDURE `sp_community_health_stats` ()
BEGIN
    SELECT
        c.id AS community_id,
        c.name AS community_name,
        COUNT(DISTINCT r.id) AS resident_count,
        COUNT(DISTINCT CASE
            WHEN hm.systolic >= 140 OR hm.diastolic >= 90 THEN r.id
        END) AS hypertension_count,
        COUNT(DISTINCT CASE
            WHEN hm.blood_sugar >= 7.0 THEN r.id
        END) AS high_sugar_count
    FROM `communities` c
    LEFT JOIN `residents` r ON r.community_id = c.id
    LEFT JOIN (
        SELECT hm1.*
        FROM `health_measurements` hm1
        INNER JOIN (
            SELECT resident_id, MAX(measured_at) AS latest
            FROM `health_measurements`
            GROUP BY resident_id
        ) hm2 ON hm1.resident_id = hm2.resident_id AND hm1.measured_at = hm2.latest
    ) hm ON r.id = hm.resident_id
    GROUP BY c.id, c.name;
END //

DELIMITER ;
