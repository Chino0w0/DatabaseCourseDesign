#pragma once

#if __has_include("../include/httplib.h")
#include "../include/httplib.h"
#elif __has_include("include/httplib.h")
#include "include/httplib.h"
#else
#include <httplib.h>
#endif

/**
 * @brief 居民自助健康档案控制器
 *
 * 注册的路由：
 *   GET /api/v1/me/profile
 *   GET /api/v1/me/measurements
 *   GET /api/v1/me/diseases
 *   GET /api/v1/me/visits
 */
class MyHealthController {
public:
  static void registerRoutes(httplib::Server &svr);
};
