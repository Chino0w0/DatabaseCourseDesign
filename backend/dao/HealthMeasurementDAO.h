#pragma once

#include <string>
#include <vector>

#include "../models/HealthMeasurement.h"
#include "../models/HealthRecord.h"

/**
 * @brief 健康测量数据访问层
 *
 * 封装对 health_measurements / health_records / health_warnings 的 SQL 操作。
 * 所有 SQL 仅写在 DAO 层。
 */
class HealthMeasurementDAO {
public:
    /**
     * @brief 查询某居民的测量历史（按测量时间倒序）
     * @param resident_id 居民 ID
     * @param limit       返回条数上限（默认 50，最大建议 200）
     * @return 测量记录列表
     */
    std::vector<HealthMeasurement> listByResidentId(int resident_id, int limit = 50);

    /**
     * @brief 新增测量记录（自动计算 BMI）
     * @param m 测量实体
     * @return 新增记录 ID
     */
    int insertMeasurement(const HealthMeasurement& m);

    /**
     * @brief 获取居民健康档案摘要
     * @param resident_id 居民 ID
     * @return 健康档案；不存在时 id=0
     */
    HealthRecord getHealthRecordByResidentId(int resident_id);

    /**
     * @brief 判断居民是否存在
     * @param resident_id 居民 ID
     * @return true 存在；false 不存在
     */
    bool residentExists(int resident_id);

    /**
     * @brief 判断用户是否存在（用于 measured_by 校验）
     * @param user_id 用户 ID
     * @return true 存在；false 不存在
     */
    bool userExists(int user_id);

    /**
     * @brief 查询某条测量记录触发的预警信息（来自 health_warnings）
     * @param measurement_id 测量记录 ID
     * @return 预警文本列表
     */
    std::vector<std::string> getWarningMessagesByMeasurementId(int measurement_id);

    /**
     * @brief 获取预警总数
     * @param only_unhandled 是否仅统计未处理预警（is_handled = 0）
     * @return 预警条数
     */
    int countWarnings(bool only_unhandled = false);

    /**
     * @brief 新增或更新居民健康档案摘要
     * @param resident_id 居民 ID
     * @param blood_type 血型
     * @param allergy_history 过敏史
     * @param family_history 家族病史
     * @param past_medical_history 既往病史
     * @return true 更新成功
     */
    bool upsertHealthRecord(int resident_id,
                            const std::string& blood_type,
                            const std::string& allergy_history,
                            const std::string& family_history,
                            const std::string& past_medical_history);
};
