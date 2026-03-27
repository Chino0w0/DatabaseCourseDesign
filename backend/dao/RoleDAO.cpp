#include "RoleDAO.h"

#include "../utils/DatabaseManager.h"

#include <cstdlib>
#include <string>

namespace {

std::string safeString(const char *value) {
  return value ? std::string(value) : std::string();
}

int safeInt(const char *value) { return value ? std::atoi(value) : 0; }

} // namespace

std::optional<Role> RoleDAO::findById(int roleId) const {
  DatabaseManager &db = DatabaseManager::getInstance();

  const std::string sql = "SELECT id, role_name, COALESCE(description, '') "
                          "FROM roles WHERE id = " +
                          std::to_string(roleId) + " LIMIT 1";

  MYSQL_RES *result = db.query(sql);
  if (!result) {
    return std::nullopt;
  }

  MYSQL_ROW row = mysql_fetch_row(result);
  if (!row) {
    mysql_free_result(result);
    return std::nullopt;
  }

  Role role;
  role.id = safeInt(row[0]);
  role.roleName = safeString(row[1]);
  role.description = safeString(row[2]);

  mysql_free_result(result);
  return role;
}

bool RoleDAO::exists(int roleId) const { return findById(roleId).has_value(); }
