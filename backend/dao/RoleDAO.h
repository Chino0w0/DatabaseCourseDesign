#pragma once

#include "../models/Role.h"

#include <optional>

/**
 * @brief 角色数据访问层
 */
class RoleDAO {
public:
  std::optional<Role> findById(int roleId) const;
  bool exists(int roleId) const;
};
