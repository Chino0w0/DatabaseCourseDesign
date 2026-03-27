#include "../services/AuthService.h"

#include "../dao/UserDAO.h"

AuthResult AuthService::login(const std::string &username,
                              const std::string &password) const {
  UserDAO dao;
  AuthResult result;

  auto userOpt = dao.findByUsername(username);
  if (!userOpt.has_value()) {
    result.code = 401;
    result.msg = "用户名或密码错误";
    return result;
  }

  User user = *userOpt;
  if (user.status != 1) {
    result.code = 401;
    result.msg = "用户已被禁用";
    return result;
  }

  if (!dao.verifyPassword(user.id, password)) {
    result.code = 401;
    result.msg = "用户名或密码错误";
    return result;
  }

  result.success = true;
  result.code = 200;
  result.msg = "登录成功";
  result.user = user;
  return result;
}
