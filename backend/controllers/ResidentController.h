#pragma once

// cpp-httplib：优先使用手动放置的头文件，否则使用 FetchContent 版本
#if __has_include("../include/httplib.h")
#  include "../include/httplib.h"
#elif __has_include("include/httplib.h")
#  include "include/httplib.h"
#else
#  include <httplib.h>
#endif

/**
 * @brief 居民档案控制器
 *
 * 负责注册 /api/v1/residents 和 /api/v1/communities 路由，
 * 解析 HTTP 请求参数后调用 ResidentService，并通过 ResponseHelper
 * 发送统一格式的 JSON 响应。
 *
 * 注册的路由一览：
 *   GET    /api/v1/residents              - 居民列表（分页+搜索）
 *   GET    /api/v1/residents/:id          - 居民详情
 *   POST   /api/v1/residents              - 新增居民
 *   PUT    /api/v1/residents/:id          - 更新居民
 *   DELETE /api/v1/residents/:id          - 删除居民
 *   GET    /api/v1/communities            - 社区列表
 *   POST   /api/v1/communities            - 新增社区
 */
class ResidentController {
public:
    /**
     * @brief 向 HTTP 服务器实例注册所有居民档案相关路由
     * @param svr  cpp-httplib 服务器实例引用
     */
    static void registerRoutes(httplib::Server& svr);
};
