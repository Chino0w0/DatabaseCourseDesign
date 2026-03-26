-- ============================================================
-- 社区健康档案管理系统 — 触发器 (Triggers)
-- 课程设计要求：根据功能需求建立相应的触发器，以保证数据的一致性
-- ============================================================

USE community_health;

DELIMITER //

-- ============================================================
-- 触发器 1：新增居民时自动创建健康档案摘要记录
-- 说明：保证每位居民都有一条对应的 health_records 记录（一人一档）
-- ============================================================
CREATE TRIGGER `trg_after_resident_insert`
AFTER INSERT ON `residents`
FOR EACH ROW
BEGIN
    INSERT INTO `health_records` (resident_id)
    VALUES (NEW.id);
END //

-- ============================================================
-- 触发器 2：删除居民前记录日志（审计用途）
-- 说明：在居民被删除前，将关键信息写入审计日志表
-- ============================================================

-- 先创建审计日志表（如不存在）
CREATE TABLE IF NOT EXISTS `audit_logs` (
    `id`          INT AUTO_INCREMENT PRIMARY KEY  COMMENT '日志ID',
    `table_name`  VARCHAR(50) NOT NULL             COMMENT '操作表名',
    `operation`   VARCHAR(10) NOT NULL             COMMENT '操作类型(INSERT/UPDATE/DELETE)',
    `record_id`   INT NOT NULL                     COMMENT '记录ID',
    `old_data`    JSON DEFAULT NULL                COMMENT '操作前的数据(JSON)',
    `new_data`    JSON DEFAULT NULL                COMMENT '操作后的数据(JSON)',
    `operated_at` DATETIME DEFAULT CURRENT_TIMESTAMP COMMENT '操作时间'
) ENGINE=InnoDB COMMENT='审计日志表' //

CREATE TRIGGER `trg_before_resident_delete`
BEFORE DELETE ON `residents`
FOR EACH ROW
BEGIN
    INSERT INTO `audit_logs` (table_name, operation, record_id, old_data)
    VALUES (
        'residents',
        'DELETE',
        OLD.id,
        JSON_OBJECT(
            'name', OLD.name,
            'gender', OLD.gender,
            'id_card', OLD.id_card,
            'birth_date', OLD.birth_date,
            'phone', OLD.phone,
            'community_id', OLD.community_id,
            'address', OLD.address
        )
    );
END //

-- ============================================================
-- 触发器 3：健康测量数据插入后自动检测异常并写入预警日志
-- 说明：当血压或血糖超出正常范围时，将预警信息写入独立的预警表
-- ============================================================

-- 先创建预警记录表（如不存在）
CREATE TABLE IF NOT EXISTS `health_warnings` (
    `id`             INT AUTO_INCREMENT PRIMARY KEY  COMMENT '预警ID',
    `measurement_id` INT NOT NULL                    COMMENT '测量记录ID',
    `resident_id`    INT NOT NULL                    COMMENT '居民ID',
    `warning_type`   VARCHAR(50) NOT NULL            COMMENT '预警类型',
    `warning_msg`    VARCHAR(255) NOT NULL           COMMENT '预警内容',
    `warning_level`  ENUM('低','中','高') DEFAULT '中' COMMENT '预警等级',
    `is_handled`     TINYINT DEFAULT 0               COMMENT '是否已处理: 0否 1是',
    `created_at`     DATETIME DEFAULT CURRENT_TIMESTAMP COMMENT '创建时间',
    INDEX `idx_resident` (`resident_id`),
    INDEX `idx_unhandled` (`is_handled`, `created_at`)
) ENGINE=InnoDB COMMENT='健康预警记录表' //

CREATE TRIGGER `trg_after_measurement_insert`
AFTER INSERT ON `health_measurements`
FOR EACH ROW
BEGIN
    -- 高血压预警
    IF NEW.systolic >= 140 OR NEW.diastolic >= 90 THEN
        INSERT INTO `health_warnings` (measurement_id, resident_id, warning_type, warning_msg, warning_level)
        VALUES (
            NEW.id, NEW.resident_id, '高血压',
            CONCAT('收缩压: ', NEW.systolic, ' mmHg, 舒张压: ', NEW.diastolic, ' mmHg'),
            CASE
                WHEN NEW.systolic >= 180 OR NEW.diastolic >= 110 THEN '高'
                WHEN NEW.systolic >= 160 OR NEW.diastolic >= 100 THEN '中'
                ELSE '低'
            END
        );
    END IF;

    -- 低血压预警
    IF NEW.systolic IS NOT NULL AND (NEW.systolic <= 90 OR NEW.diastolic <= 60) THEN
        INSERT INTO `health_warnings` (measurement_id, resident_id, warning_type, warning_msg, warning_level)
        VALUES (NEW.id, NEW.resident_id, '低血压',
                CONCAT('收缩压: ', NEW.systolic, ' mmHg, 舒张压: ', NEW.diastolic, ' mmHg'), '中');
    END IF;

    -- 高血糖预警
    IF NEW.blood_sugar >= 7.0 THEN
        INSERT INTO `health_warnings` (measurement_id, resident_id, warning_type, warning_msg, warning_level)
        VALUES (
            NEW.id, NEW.resident_id, '高血糖',
            CONCAT('血糖值: ', NEW.blood_sugar, ' mmol/L'),
            IF(NEW.blood_sugar >= 11.1, '高', '中')
        );
    END IF;

    -- 低血糖预警
    IF NEW.blood_sugar IS NOT NULL AND NEW.blood_sugar <= 3.9 THEN
        INSERT INTO `health_warnings` (measurement_id, resident_id, warning_type, warning_msg, warning_level)
        VALUES (NEW.id, NEW.resident_id, '低血糖',
                CONCAT('血糖值: ', NEW.blood_sugar, ' mmol/L'), '高');
    END IF;

    -- 心率异常预警
    IF NEW.heart_rate IS NOT NULL AND (NEW.heart_rate > 100 OR NEW.heart_rate < 60) THEN
        INSERT INTO `health_warnings` (measurement_id, resident_id, warning_type, warning_msg, warning_level)
        VALUES (NEW.id, NEW.resident_id, '心率异常',
                CONCAT('心率: ', NEW.heart_rate, ' 次/分'),
                IF(NEW.heart_rate > 120 OR NEW.heart_rate < 50, '高', '低'));
    END IF;
END //

-- ============================================================
-- 触发器 4：修改居民信息时记录审计日志
-- 说明：居民关键信息被修改时，自动记录变更前后的数据
-- ============================================================
CREATE TRIGGER `trg_after_resident_update`
AFTER UPDATE ON `residents`
FOR EACH ROW
BEGIN
    IF OLD.name != NEW.name OR OLD.id_card != NEW.id_card
       OR OLD.phone != NEW.phone OR OLD.community_id != NEW.community_id THEN
        INSERT INTO `audit_logs` (table_name, operation, record_id, old_data, new_data)
        VALUES (
            'residents',
            'UPDATE',
            NEW.id,
            JSON_OBJECT('name', OLD.name, 'id_card', OLD.id_card, 'phone', OLD.phone, 'community_id', OLD.community_id),
            JSON_OBJECT('name', NEW.name, 'id_card', NEW.id_card, 'phone', NEW.phone, 'community_id', NEW.community_id)
        );
    END IF;
END //

DELIMITER ;
