#pragma once

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
  std::vector<HealthMeasurement> listByResidentId(int resident_id,
                                                  int limit = 50);

  /**
   * @brief 新增测量记录（自动计算 BMI）
   * @param m 测量实体
   * @return 新增记录 ID
   */
  int insertMeasurement(const HealthMeasurement &m);

  /**
   * @brief 根据测量 ID 获取单条测量记录
   * @param measurement_id 测量记录 ID
   * @return 测量记录；不存在时 id=0
   */
  HealthMeasurement getMeasurementById(int measurement_id);

  /**
   * @brief 更新测量记录并重新计算 BMI / 预警
   * @param measurement_id 测量记录 ID
   * @param m              测量实体
   * @return true 更新成功；false 记录不存在
   */
  bool updateMeasurement(int measurement_id, const HealthMeasurement &m);

  /**
   * @brief 获取居民健康档案摘要
   * @param resident_id 居民 ID
   * @return 健康档案；不存在时 id=0
   */
  HealthRecord getHealthRecordByResidentId(int resident_id);

  /**
   * @brief 新增或更新居民健康档案摘要
   * @param resident_id 居民 ID
   * @param record      健康档案摘要
   * @return true 保存成功；false 居民不存在
   */
  bool upsertHealthRecordByResidentId(int resident_id,
                                      const HealthRecord &record);

  /**
   * @brief 统计“最新一次测量仍处于异常”的居民数
   * @return 当前异常预警居民数
   */
  int countWarnings();

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
  std::vector<std::string>
  getWarningMessagesByMeasurementId(int measurement_id);
};
