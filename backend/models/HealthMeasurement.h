#pragma once

#include <string>

/**
 * @brief 居民健康测量实体，对应数据库 health_measurements 表
 */
struct HealthMeasurement {
    int         id{0};                    ///< 测量 ID（主键）
    int         resident_id{0};           ///< 居民 ID
    int         systolic{-1};             ///< 收缩压（mmHg，-1 表示未填）
    int         diastolic{-1};            ///< 舒张压（mmHg，-1 表示未填）
    double      blood_sugar{-1.0};        ///< 血糖（mmol/L，-1 表示未填）
    int         heart_rate{-1};           ///< 心率（次/分，-1 表示未填）
    double      height{-1.0};             ///< 身高（cm，-1 表示未填）
    double      weight{-1.0};             ///< 体重（kg，-1 表示未填）
    double      bmi{-1.0};                ///< BMI 指数（-1 表示未计算）
    std::string notes;                    ///< 备注
    int         measured_by{0};           ///< 测量人用户 ID
    std::string measured_by_name;         ///< 测量人姓名（JOIN 填充）
    std::string measured_at;              ///< 测量时间
};
