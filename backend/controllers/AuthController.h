#pragma once

#include "../include/httplib.h"

/**
 * @brief 认证控制器
 */
class AuthController {
public:
  static void registerRoutes(httplib::Server &svr);
};
