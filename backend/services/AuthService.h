#pragma once

#include "../models/User.h"

#include <string>

/**
 * @brief 登录认证结果
 */
struct AuthResult {
  bool success = false;
  int code = 401;
  std::string msg = "用户名或密码错误";
  std::string token;
  User user;
};

/**
 * @brief 认证服务层
 */
class AuthService {
public:
  AuthResult login(const std::string &username,
                   const std::string &password) const;
};
