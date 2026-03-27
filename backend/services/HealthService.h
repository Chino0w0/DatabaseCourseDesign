#pragma once

#include <string>

#if __has_include("../include/json.hpp")
#  include "../include/json.hpp"
#elif __has_include("include/json.hpp")
#  include "include/json.hpp"
#else
#  include <nlohmann/json.hpp>
#endif

using json = nlohmann::json;

/**
 * @brief 健康测量与预警业务层
 */
class HealthService {
public:
    /**
     * @brief 查询居民测量历史
     * @param resident_id 居民 ID
     * @param limit       返回条数上限
     * @return JSON 数组；参数非法时返回带 error 的对象
     */
    json getMeasurements(int resident_id, int limit = 50);

    /**
     * @brief 录入一条测量数据
     * @param body 请求体
     * @return 成功返回新增记录及预警信息；失败返回带 error 的对象
     */
    json createMeasurement(const json& body);

    /**
     * @brief 查询居民健康档案摘要
     * @param resident_id 居民 ID
     * @return 健康档案对象；不存在时返回 null；参数非法时返回带 error 的对象
     */
    json getHealthRecord(int resident_id);
};
