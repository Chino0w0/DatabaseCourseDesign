#pragma once

#if __has_include("include/httplib.h")
#include "include/httplib.h"
#else
#include <httplib.h>
#endif

/**
 * @brief 用户管理控制器
 */
class UserController {
public:
  static void registerRoutes(httplib::Server &svr);
};
