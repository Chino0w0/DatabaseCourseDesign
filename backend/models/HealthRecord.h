#pragma once

#include <string>

/**
 * @brief 居民健康档案摘要实体，对应数据库 health_records 表
 */
struct HealthRecord {
    int         id{0};                    ///< 档案 ID（主键）
    int         resident_id{0};           ///< 居民 ID（一人一档）
    std::string blood_type;               ///< 血型
    std::string allergy_history;          ///< 过敏史
    std::string family_history;           ///< 家族病史
    std::string past_medical_history;     ///< 既往病史
    std::string created_at;               ///< 创建时间
    std::string updated_at;               ///< 更新时间
};
