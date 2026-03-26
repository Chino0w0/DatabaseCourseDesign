-- ============================================================
-- 社区健康档案管理系统 — 数据库初始化脚本
-- 数据库：MySQL 8.0+
-- 字符集：utf8mb4
-- 设计范式：第三范式 (3NF)
-- ============================================================

-- 创建数据库
CREATE DATABASE IF NOT EXISTS community_health
  DEFAULT CHARACTER SET utf8mb4
  DEFAULT COLLATE utf8mb4_unicode_ci;

USE community_health;

-- ============================================================
-- 表 1：系统用户表
-- ============================================================
CREATE TABLE IF NOT EXISTS `users` (
  `id`            INT           AUTO_INCREMENT PRIMARY KEY  COMMENT '用户ID',
  `username`      VARCHAR(50)   NOT NULL UNIQUE             COMMENT '登录账号',
  `password_hash` VARCHAR(255)  NOT NULL                    COMMENT '密码哈希值',
  `real_name`     VARCHAR(50)   NOT NULL                    COMMENT '真实姓名',
  `phone`         VARCHAR(20)   DEFAULT NULL                COMMENT '手机号',
  `status`        TINYINT       NOT NULL DEFAULT 1          COMMENT '状态: 1启用 0禁用',
  `created_at`    DATETIME      DEFAULT CURRENT_TIMESTAMP   COMMENT '创建时间',
  `updated_at`    DATETIME      DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP COMMENT '更新时间'
) ENGINE=InnoDB COMMENT='系统用户表';

-- ============================================================
-- 表 2：角色表
-- ============================================================
CREATE TABLE IF NOT EXISTS `roles` (
  `id`          INT           AUTO_INCREMENT PRIMARY KEY  COMMENT '角色ID',
  `role_name`   VARCHAR(30)   NOT NULL UNIQUE             COMMENT '角色名称',
  `description` VARCHAR(100)  DEFAULT NULL                COMMENT '角色描述'
) ENGINE=InnoDB COMMENT='角色表';

-- ============================================================
-- 表 3：用户角色关联表 (多对多)
-- ============================================================
CREATE TABLE IF NOT EXISTS `user_roles` (
  `id`      INT AUTO_INCREMENT PRIMARY KEY  COMMENT '主键',
  `user_id` INT NOT NULL                    COMMENT '用户ID',
  `role_id` INT NOT NULL                    COMMENT '角色ID',
  UNIQUE KEY `uk_user_role` (`user_id`, `role_id`),
  CONSTRAINT `fk_ur_user` FOREIGN KEY (`user_id`) REFERENCES `users`(`id`) ON DELETE CASCADE,
  CONSTRAINT `fk_ur_role` FOREIGN KEY (`role_id`) REFERENCES `roles`(`id`) ON DELETE CASCADE
) ENGINE=InnoDB COMMENT='用户角色关联表';

-- ============================================================
-- 表 4：社区信息表
-- ============================================================
CREATE TABLE IF NOT EXISTS `communities` (
  `id`            INT           AUTO_INCREMENT PRIMARY KEY  COMMENT '社区ID',
  `name`          VARCHAR(100)  NOT NULL                    COMMENT '社区名称',
  `address`       VARCHAR(255)  DEFAULT NULL                COMMENT '社区地址',
  `contact_phone` VARCHAR(20)   DEFAULT NULL                COMMENT '联系电话',
  `created_at`    DATETIME      DEFAULT CURRENT_TIMESTAMP   COMMENT '创建时间'
) ENGINE=InnoDB COMMENT='社区信息表';

-- ============================================================
-- 表 5：居民基本信息表
-- ============================================================
CREATE TABLE IF NOT EXISTS `residents` (
  `id`                INT           AUTO_INCREMENT PRIMARY KEY  COMMENT '居民ID',
  `name`              VARCHAR(50)   NOT NULL                    COMMENT '姓名',
  `gender`            ENUM('男','女') NOT NULL                  COMMENT '性别',
  `id_card`           VARCHAR(18)   NOT NULL UNIQUE             COMMENT '身份证号',
  `birth_date`        DATE          NOT NULL                    COMMENT '出生日期',
  `phone`             VARCHAR(20)   DEFAULT NULL                COMMENT '联系电话',
  `community_id`      INT           DEFAULT NULL                COMMENT '所属社区ID',
  `address`           VARCHAR(255)  DEFAULT NULL                COMMENT '详细地址',
  `emergency_contact` VARCHAR(50)   DEFAULT NULL                COMMENT '紧急联系人',
  `emergency_phone`   VARCHAR(20)   DEFAULT NULL                COMMENT '紧急联系人电话',
  `created_at`        DATETIME      DEFAULT CURRENT_TIMESTAMP   COMMENT '建档时间',
  `updated_at`        DATETIME      DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP COMMENT '更新时间',
  INDEX `idx_community` (`community_id`),
  INDEX `idx_name` (`name`),
  CONSTRAINT `fk_res_community` FOREIGN KEY (`community_id`) REFERENCES `communities`(`id`) ON DELETE SET NULL
) ENGINE=InnoDB COMMENT='居民基本信息表';

-- ============================================================
-- 表 6：健康档案摘要表 (一人一档)
-- ============================================================
CREATE TABLE IF NOT EXISTS `health_records` (
  `id`                   INT     AUTO_INCREMENT PRIMARY KEY  COMMENT '档案ID',
  `resident_id`          INT     NOT NULL UNIQUE             COMMENT '居民ID(一人一档)',
  `blood_type`           VARCHAR(10)  DEFAULT NULL           COMMENT '血型',
  `allergy_history`      TEXT         DEFAULT NULL           COMMENT '过敏史',
  `family_history`       TEXT         DEFAULT NULL           COMMENT '家族病史',
  `past_medical_history` TEXT         DEFAULT NULL           COMMENT '既往病史',
  `created_at`           DATETIME     DEFAULT CURRENT_TIMESTAMP  COMMENT '创建时间',
  `updated_at`           DATETIME     DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP COMMENT '更新时间',
  CONSTRAINT `fk_hr_resident` FOREIGN KEY (`resident_id`) REFERENCES `residents`(`id`) ON DELETE CASCADE
) ENGINE=InnoDB COMMENT='健康档案摘要表';

-- ============================================================
-- 表 7：健康测量数据表
-- ============================================================
CREATE TABLE IF NOT EXISTS `health_measurements` (
  `id`          INT            AUTO_INCREMENT PRIMARY KEY  COMMENT '测量ID',
  `resident_id` INT            NOT NULL                    COMMENT '居民ID',
  `systolic`    INT            DEFAULT NULL                COMMENT '收缩压(mmHg)',
  `diastolic`   INT            DEFAULT NULL                COMMENT '舒张压(mmHg)',
  `blood_sugar` DECIMAL(5,2)   DEFAULT NULL                COMMENT '血糖(mmol/L)',
  `heart_rate`  INT            DEFAULT NULL                COMMENT '心率(次/分)',
  `height`      DECIMAL(5,1)   DEFAULT NULL                COMMENT '身高(cm)',
  `weight`      DECIMAL(5,1)   DEFAULT NULL                COMMENT '体重(kg)',
  `bmi`         DECIMAL(4,1)   DEFAULT NULL                COMMENT 'BMI指数',
  `notes`       TEXT           DEFAULT NULL                COMMENT '备注',
  `measured_by` INT            DEFAULT NULL                COMMENT '测量人(用户ID)',
  `measured_at` DATETIME       DEFAULT CURRENT_TIMESTAMP   COMMENT '测量时间',
  INDEX `idx_resident_time` (`resident_id`, `measured_at`),
  CONSTRAINT `fk_hm_resident` FOREIGN KEY (`resident_id`) REFERENCES `residents`(`id`) ON DELETE CASCADE,
  CONSTRAINT `fk_hm_user`     FOREIGN KEY (`measured_by`)  REFERENCES `users`(`id`) ON DELETE SET NULL
) ENGINE=InnoDB COMMENT='健康测量数据表';

-- ============================================================
-- 表 8：慢性病字典表
-- ============================================================
CREATE TABLE IF NOT EXISTS `chronic_diseases` (
  `id`           INT           AUTO_INCREMENT PRIMARY KEY  COMMENT '疾病ID',
  `disease_name` VARCHAR(100)  NOT NULL UNIQUE             COMMENT '疾病名称',
  `disease_code` VARCHAR(20)   DEFAULT NULL                COMMENT 'ICD编码',
  `category`     VARCHAR(50)   DEFAULT NULL                COMMENT '疾病分类',
  `description`  TEXT          DEFAULT NULL                COMMENT '疾病描述'
) ENGINE=InnoDB COMMENT='慢性病字典表';

-- ============================================================
-- 表 9：居民慢性病关联表
-- ============================================================
CREATE TABLE IF NOT EXISTS `resident_diseases` (
  `id`             INT   AUTO_INCREMENT PRIMARY KEY  COMMENT '主键',
  `resident_id`    INT   NOT NULL                    COMMENT '居民ID',
  `disease_id`     INT   NOT NULL                    COMMENT '疾病ID',
  `diagnosed_date` DATE  DEFAULT NULL                COMMENT '确诊日期',
  `status`         ENUM('治疗中','已痊愈','控制中') DEFAULT '治疗中' COMMENT '状态',
  `notes`          TEXT  DEFAULT NULL                COMMENT '备注',
  UNIQUE KEY `uk_resident_disease` (`resident_id`, `disease_id`),
  CONSTRAINT `fk_rd_resident` FOREIGN KEY (`resident_id`) REFERENCES `residents`(`id`) ON DELETE CASCADE,
  CONSTRAINT `fk_rd_disease`  FOREIGN KEY (`disease_id`)  REFERENCES `chronic_diseases`(`id`) ON DELETE CASCADE
) ENGINE=InnoDB COMMENT='居民慢性病关联表';

-- ============================================================
-- 表 10：随访记录表
-- ============================================================
CREATE TABLE IF NOT EXISTS `visit_logs` (
  `id`              INT          AUTO_INCREMENT PRIMARY KEY  COMMENT '记录ID',
  `resident_id`     INT          NOT NULL                    COMMENT '被随访居民ID',
  `visitor_user_id` INT          NOT NULL                    COMMENT '随访人员(用户ID)',
  `visit_type`      VARCHAR(30)  DEFAULT NULL                COMMENT '随访方式(上门/电话/门诊)',
  `visit_date`      DATE         NOT NULL                    COMMENT '随访日期',
  `content`         TEXT         DEFAULT NULL                COMMENT '随访内容',
  `next_visit_date` DATE         DEFAULT NULL                COMMENT '下次随访日期',
  `created_at`      DATETIME     DEFAULT CURRENT_TIMESTAMP   COMMENT '记录创建时间',
  INDEX `idx_resident_date` (`resident_id`, `visit_date`),
  CONSTRAINT `fk_vl_resident` FOREIGN KEY (`resident_id`)     REFERENCES `residents`(`id`) ON DELETE CASCADE,
  CONSTRAINT `fk_vl_user`     FOREIGN KEY (`visitor_user_id`) REFERENCES `users`(`id`) ON DELETE RESTRICT
) ENGINE=InnoDB COMMENT='随访记录表';

-- ============================================================
-- 初始数据：角色 & 管理员账号
-- ============================================================

-- 插入默认角色
INSERT INTO `roles` (`role_name`, `description`) VALUES
  ('管理员', '系统管理员，拥有全部权限'),
  ('医生', '社区医生，可录入健康数据和随访'),
  ('护士', '社区护士，可录入健康测量数据');

-- 插入默认管理员账号（密码: 123456 的 SHA-256 哈希值简化示例）
-- 实际项目中应使用 bcrypt 等安全哈希算法
INSERT INTO `users` (`username`, `password_hash`, `real_name`, `phone`) VALUES
  ('admin', '8d969eef6ecad3c29a3a629280e686cf0c3f5d5a86aff3ca12020c923adc6c92', '系统管理员', '13800000000');

-- 为管理员分配角色
INSERT INTO `user_roles` (`user_id`, `role_id`) VALUES (1, 1);

-- 插入示例社区
INSERT INTO `communities` (`name`, `address`, `contact_phone`) VALUES
  ('阳光社区', '城东区阳光路100号', '010-88881111'),
  ('和谐社区', '城西区和谐大道200号', '010-88882222'),
  ('幸福社区', '城南区幸福街300号', '010-88883333');

-- 插入示例慢性病数据
INSERT INTO `chronic_diseases` (`disease_name`, `disease_code`, `category`, `description`) VALUES
  ('高血压', 'I10', '心血管疾病', '原发性高血压，血压持续升高'),
  ('2型糖尿病', 'E11', '内分泌疾病', '胰岛素分泌不足或利用障碍'),
  ('冠心病', 'I25', '心血管疾病', '冠状动脉粥样硬化性心脏病'),
  ('慢性阻塞性肺疾病', 'J44', '呼吸系统疾病', '慢性支气管炎和/或肺气肿'),
  ('脑卒中', 'I64', '脑血管疾病', '脑血管意外，包括脑梗死和脑出血');
