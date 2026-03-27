#pragma once

#if __has_include("../include/httplib.h")
#  include "../include/httplib.h"
#elif __has_include("include/httplib.h")
#  include "include/httplib.h"
#else
#  include <httplib.h>
#endif

/**
 * @brief 健康测量控制器
 *
 * 注册的路由：
 *   GET  /api/v1/health/measurements
 *   POST /api/v1/health/measurements
 *   GET  /api/v1/health/records/:resident_id
 */
class HealthController {
public:
    static void registerRoutes(httplib::Server& svr);
};
