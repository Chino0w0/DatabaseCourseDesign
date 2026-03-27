#include "UserDAO.h"

#include "utils/DatabaseManager.h"

#include <cstdlib>
#include <sstream>
#include <stdexcept>
#include <vector>

namespace {

std::string safeString(const char *value) {
  return value ? std::string(value) : std::string();
}

int safeInt(const char *value) { return value ? std::atoi(value) : 0; }

std::string quote(DatabaseManager &db, const std::string &value) {
  return "'" + db.escape(value) + "'";
}

std::string nullableQuote(DatabaseManager &db,
                          const std::optional<std::string> &value) {
  if (!value.has_value()) {
    return "NULL";
  }
  return quote(db, *value);
}

User mapUserRow(MYSQL_ROW row) {
  User user;
  user.id = safeInt(row[0]);
  user.username = safeString(row[1]);
  user.passwordHash = safeString(row[2]);
  user.realName = safeString(row[3]);
  user.phone = safeString(row[4]);
  user.status = safeInt(row[5]);
  user.createdAt = safeString(row[6]);
  user.updatedAt = safeString(row[7]);
  user.roleId = safeInt(row[8]);
  user.roleName = safeString(row[9]);
  return user;
}

std::string selectUserBaseSql() {
  return "SELECT "
         "u.id, "
         "u.username, "
         "u.password_hash, "
         "u.real_name, "
         "COALESCE(u.phone, ''), "
         "u.status, "
         "COALESCE(DATE_FORMAT(u.created_at, '%Y-%m-%d %H:%i:%s'), ''), "
         "COALESCE(DATE_FORMAT(u.updated_at, '%Y-%m-%d %H:%i:%s'), ''), "
         "COALESCE(ur.role_id, 0), "
         "COALESCE(r.role_name, '') "
         "FROM users u "
         "LEFT JOIN user_roles ur ON u.id = ur.user_id "
         "LEFT JOIN roles r ON ur.role_id = r.id ";
}

} // namespace

std::optional<User> UserDAO::findById(int userId) const {
  DatabaseManager &db = DatabaseManager::getInstance();
  const std::string sql = selectUserBaseSql() +
                          "WHERE u.id = " + std::to_string(userId) + " LIMIT 1";

  MYSQL_RES *result = db.query(sql);
  if (!result) {
    return std::nullopt;
  }

  MYSQL_ROW row = mysql_fetch_row(result);
  if (!row) {
    mysql_free_result(result);
    return std::nullopt;
  }

  User user = mapUserRow(row);
  mysql_free_result(result);
  return user;
}

std::optional<User> UserDAO::findByUsername(const std::string &username) const {
  DatabaseManager &db = DatabaseManager::getInstance();
  const std::string sql = selectUserBaseSql() +
                          "WHERE u.username = " + quote(db, username) +
                          " LIMIT 1";

  MYSQL_RES *result = db.query(sql);
  if (!result) {
    return std::nullopt;
  }

  MYSQL_ROW row = mysql_fetch_row(result);
  if (!row) {
    mysql_free_result(result);
    return std::nullopt;
  }

  User user = mapUserRow(row);
  mysql_free_result(result);
  return user;
}

bool UserDAO::verifyPassword(int userId,
                             const std::string &plainPassword) const {
  DatabaseManager &db = DatabaseManager::getInstance();
  const std::string sql =
      "SELECT id FROM users WHERE id = " + std::to_string(userId) +
      " AND password_hash = SHA2(" + quote(db, plainPassword) +
      ", 256) LIMIT 1";

  MYSQL_RES *result = db.query(sql);
  if (!result) {
    return false;
  }

  const bool matched = mysql_fetch_row(result) != nullptr;
  mysql_free_result(result);
  return matched;
}

std::vector<User> UserDAO::listUsers(int page, int size, int &total) const {
  DatabaseManager &db = DatabaseManager::getInstance();
  total = 0;

  {
    MYSQL_RES *countResult = db.query("SELECT COUNT(*) FROM users");
    if (countResult) {
      MYSQL_ROW row = mysql_fetch_row(countResult);
      if (row && row[0]) {
        total = std::atoi(row[0]);
      }
      mysql_free_result(countResult);
    }
  }

  const int safePage = page <= 0 ? 1 : page;
  const int safeSize = size <= 0 ? 10 : size;
  const int safeOffset = (safePage - 1) * safeSize;

  const std::string sql = selectUserBaseSql() +
                          "ORDER BY u.id ASC "
                          "LIMIT " +
                          std::to_string(safeOffset) + ", " +
                          std::to_string(safeSize);

  MYSQL_RES *result = db.query(sql);
  std::vector<User> users;
  if (!result) {
    return users;
  }

  MYSQL_ROW row;
  while ((row = mysql_fetch_row(result)) != nullptr) {
    users.push_back(mapUserRow(row));
  }

  mysql_free_result(result);
  return users;
}

bool UserDAO::usernameExists(const std::string &username,
                             int excludeUserId) const {
  DatabaseManager &db = DatabaseManager::getInstance();

  std::string sql =
      "SELECT id FROM users WHERE username = " + quote(db, username);
  if (excludeUserId > 0) {
    sql += " AND id <> " + std::to_string(excludeUserId);
  }
  sql += " LIMIT 1";

  MYSQL_RES *result = db.query(sql);
  if (!result) {
    return false;
  }

  const bool exists = mysql_fetch_row(result) != nullptr;
  mysql_free_result(result);
  return exists;
}

int UserDAO::createUser(const std::string &username,
                        const std::string &plainPassword,
                        const std::string &realName,
                        const std::optional<std::string> &phone, int roleId,
                        int status) const {
  DatabaseManager &db = DatabaseManager::getInstance();

  try {
    db.execute("START TRANSACTION");

    const std::string insertUserSql =
        "INSERT INTO users (username, password_hash, real_name, phone, status) "
        "VALUES (" +
        quote(db, username) + ", " + "SHA2(" + quote(db, plainPassword) +
        ", 256), " + quote(db, realName) + ", " + nullableQuote(db, phone) +
        ", " + std::to_string(status) + ")";

    db.execute(insertUserSql);
    const int newUserId = static_cast<int>(db.lastInsertId());

    const std::string insertUserRoleSql =
        "INSERT INTO user_roles (user_id, role_id) VALUES (" +
        std::to_string(newUserId) + ", " + std::to_string(roleId) + ")";

    db.execute(insertUserRoleSql);
    db.execute("COMMIT");
    return newUserId;
  } catch (...) {
    try {
      db.execute("ROLLBACK");
    } catch (...) {
    }
    throw;
  }
}

bool UserDAO::updateUser(
    int userId, const std::optional<std::string> &realName,
    const std::optional<std::string> &phone, const std::optional<int> &status,
    const std::optional<int> &roleId,
    const std::optional<std::string> &plainPassword) const {
  DatabaseManager &db = DatabaseManager::getInstance();
  if (!findById(userId).has_value()) {
    return false;
  }

  try {
    db.execute("START TRANSACTION");

    std::vector<std::string> assignments;
    if (realName.has_value()) {
      assignments.push_back("real_name = " + quote(db, *realName));
    }
    if (phone.has_value()) {
      assignments.push_back("phone = " + quote(db, *phone));
    }
    if (status.has_value()) {
      assignments.push_back("status = " + std::to_string(*status));
    }
    if (plainPassword.has_value()) {
      assignments.push_back("password_hash = SHA2(" +
                            quote(db, *plainPassword) + ", 256)");
    }

    if (!assignments.empty()) {
      std::ostringstream sql;
      sql << "UPDATE users SET ";
      for (std::size_t i = 0; i < assignments.size(); ++i) {
        if (i > 0) {
          sql << ", ";
        }
        sql << assignments[i];
      }
      sql << " WHERE id = " << userId;
      db.execute(sql.str());
    }

    if (roleId.has_value()) {
      db.execute("DELETE FROM user_roles WHERE user_id = " +
                 std::to_string(userId));
      db.execute("INSERT INTO user_roles (user_id, role_id) VALUES (" +
                 std::to_string(userId) + ", " + std::to_string(*roleId) + ")");
    }

    db.execute("COMMIT");
    return true;
  } catch (...) {
    try {
      db.execute("ROLLBACK");
    } catch (...) {
    }
    throw;
  }
}

bool UserDAO::softDeleteUser(int userId) const {
  DatabaseManager &db = DatabaseManager::getInstance();
  if (!findById(userId).has_value()) {
    return false;
  }

  db.execute("UPDATE users SET status = 0 WHERE id = " +
             std::to_string(userId));
  return true;
}
