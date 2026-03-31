#pragma once

#include <string>

#if __has_include("../include/json.hpp")
#include "../include/json.hpp"
#elif __has_include("include/json.hpp")
#include "include/json.hpp"
#else
#include <nlohmann/json.hpp>
#endif

using json = nlohmann::json;

/**
 * @brief 居民自助健康档案业务层
 *
 * 基于当前登录用户绑定的 resident_id 返回“本人数据”，
 * 不允许前端自行传入任意 resident_id。
 */
class MyHealthService {
public:
  json getProfile(int residentId);
  json getMeasurements(int residentId, int limit = 50);
  json getDiseases(int residentId);
  json getVisits(int residentId);
};
