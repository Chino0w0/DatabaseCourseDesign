#pragma once

#include "../models/User.h"

#include <optional>
#include <string>
#include <vector>

/**
 * @brief 用户数据访问层
 */
class UserDAO {
public:
  std::optional<User> findById(int userId) const;
  std::optional<User> findByUsername(const std::string &username) const;
  bool verifyPassword(int userId, const std::string &plainPassword) const;

  std::vector<User> listUsers(int page, int size, int &total) const;
  bool usernameExists(const std::string &username, int excludeUserId = 0) const;

  int createUser(const std::string &username, const std::string &plainPassword,
                 const std::string &realName,
                 const std::optional<std::string> &phone, int roleId,
                 int status = 1) const;

  bool updateUser(int userId, const std::optional<std::string> &realName,
                  const std::optional<std::string> &phone,
                  const std::optional<int> &status,
                  const std::optional<int> &roleId,
                  const std::optional<std::string> &plainPassword) const;

  bool softDeleteUser(int userId) const;
};
