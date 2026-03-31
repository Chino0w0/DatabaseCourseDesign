#pragma once

#include <string>

/**
 * @brief 系统用户实体
 */
struct User {
  int id = 0;
  std::string username;
  std::string passwordHash;
  std::string realName;
  std::string phone;
  int status = 1;
  std::string createdAt;
  std::string updatedAt;
  int roleId = 0;
  std::string roleName;
  int residentId = 0;
};
