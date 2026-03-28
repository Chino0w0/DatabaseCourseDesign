#include "UserController.h"

#include "dao/RoleDAO.h"
#include "dao/UserDAO.h"
#include "utils/AuthSessionManager.h"
#include "utils/ResponseHelper.h"

#include <algorithm>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <optional>
#include <string>
#include <vector>

namespace {

void sendOk(httplib::Response &res, const json &data,
            const std::string &msg = "操作成功") {
  ResponseHelper::setCorsHeaders(res);
  ResponseHelper::ok(res, data, msg);
}

void sendFail(httplib::Response &res, int code, const std::string &msg) {
  ResponseHelper::setCorsHeaders(res);
  ResponseHelper::fail(res, code, msg);
}

void logServerError(const char *context, const std::exception &e) {
  std::cerr << "[UserController] " << context << ": " << e.what() << std::endl;
}

bool parseJsonBody(const httplib::Request &req, httplib::Response &res,
                   json &body) {
  try {
    body = json::parse(req.body);
    if (!body.is_object()) {
      sendFail(res, 400, "请求体必须是 JSON 对象");
      return false;
    }
    return true;
  } catch (...) {
    sendFail(res, 400, "请求体不是合法 JSON");
    return false;
  }
}

bool hasValidStringField(const json &body, const std::string &key) {
  return body.contains(key) && body[key].is_string() &&
         !body[key].get<std::string>().empty();
}

bool hasValidIntegerField(const json &body, const std::string &key) {
  return body.contains(key) && body[key].is_number_integer();
}

int parsePositiveInt(const std::string &value, int fallback) {
  const int parsed = std::atoi(value.c_str());
  return parsed > 0 ? parsed : fallback;
}

const std::vector<int> kAdminRoleIds{1};

json userToJson(const User &user) {
  return json{{"id", user.id},
              {"username", user.username},
              {"real_name", user.realName},
              {"phone", user.phone.empty() ? json(nullptr) : json(user.phone)},
              {"status", user.status},
              {"role_id", user.roleId},
              {"role_name", user.roleName},
              {"created_at",
               user.createdAt.empty() ? json(nullptr) : json(user.createdAt)},
              {"updated_at",
               user.updatedAt.empty() ? json(nullptr) : json(user.updatedAt)}};
}

} // namespace

void UserController::registerRoutes(httplib::Server &svr) {
  svr.Get("/api/v1/users", [](const httplib::Request &req,
                              httplib::Response &res) {
    auto currentUser = AuthSessionManager::requireUser(req, res, kAdminRoleIds);
    if (!currentUser.has_value()) {
      return;
    }

    try {
      const int page = req.has_param("page")
                           ? parsePositiveInt(req.get_param_value("page"), 1)
                           : 1;
      const int size =
          req.has_param("size")
              ? std::min(parsePositiveInt(req.get_param_value("size"), 10), 100)
              : 10;

      UserDAO userDao;
      int total = 0;
      std::vector<User> users = userDao.listUsers(page, size, total);

      json list = json::array();
      for (const auto &user : users) {
        list.push_back(userToJson(user));
      }

      sendOk(
          res,
          json{
              {"total", total}, {"page", page}, {"size", size}, {"list", list}},
          "查询成功");
    } catch (const std::exception &e) {
      logServerError("listUsers", e);
      sendFail(res, 500, "查询用户列表失败，请稍后重试");
    }
  });

  svr.Post("/api/v1/users", [](const httplib::Request &req,
                               httplib::Response &res) {
    auto currentUser = AuthSessionManager::requireUser(req, res, kAdminRoleIds);
    if (!currentUser.has_value()) {
      return;
    }

    try {
      json body;
      if (!parseJsonBody(req, res, body)) {
        return;
      }

      if (!hasValidStringField(body, "username") ||
          !hasValidStringField(body, "password") ||
          !hasValidStringField(body, "real_name") ||
          !hasValidIntegerField(body, "role_id")) {
        sendFail(res, 400,
                 "缺少必要字段：username、password、real_name、role_id");
        return;
      }

      const std::string username = body["username"].get<std::string>();
      const std::string password = body["password"].get<std::string>();
      const std::string realName = body["real_name"].get<std::string>();
      const int roleId = body["role_id"].get<int>();
      const int status =
          hasValidIntegerField(body, "status") ? body["status"].get<int>() : 1;

      if (status != 0 && status != 1) {
        sendFail(res, 400, "status 只能为 0 或 1");
        return;
      }

      std::optional<std::string> phone = std::nullopt;
      if (body.contains("phone")) {
        if (!body["phone"].is_string()) {
          sendFail(res, 400, "phone 必须为字符串");
          return;
        }
        phone = body["phone"].get<std::string>();
      }

      RoleDAO roleDao;
      if (!roleDao.exists(roleId)) {
        sendFail(res, 400, "角色不存在");
        return;
      }

      UserDAO userDao;
      if (userDao.usernameExists(username)) {
        sendFail(res, 400, "用户名已存在");
        return;
      }

      const int newUserId = userDao.createUser(username, password, realName,
                                               phone, roleId, status);
      auto newUser = userDao.findById(newUserId);
      if (!newUser.has_value()) {
        sendFail(res, 500, "用户创建成功但读取结果失败");
        return;
      }

      sendOk(res, userToJson(*newUser), "创建用户成功");
    } catch (const std::exception &e) {
      logServerError("createUser", e);
      sendFail(res, 500, "创建用户失败，请稍后重试");
    }
  });

  svr.Put(R"(/api/v1/users/(\d+))", [](const httplib::Request &req,
                                       httplib::Response &res) {
    auto currentUser = AuthSessionManager::requireUser(req, res, kAdminRoleIds);
    if (!currentUser.has_value()) {
      return;
    }

    try {
      const int userId = std::atoi(std::string(req.matches[1]).c_str());
      json body;
      if (!parseJsonBody(req, res, body)) {
        return;
      }

      std::optional<std::string> realName = std::nullopt;
      std::optional<std::string> phone = std::nullopt;
      std::optional<int> status = std::nullopt;
      std::optional<int> roleId = std::nullopt;
      std::optional<std::string> password = std::nullopt;

      if (body.contains("real_name")) {
        if (!body["real_name"].is_string() ||
            body["real_name"].get<std::string>().empty()) {
          sendFail(res, 400, "real_name 必须为非空字符串");
          return;
        }
        realName = body["real_name"].get<std::string>();
      }

      if (body.contains("phone")) {
        if (!body["phone"].is_string()) {
          sendFail(res, 400, "phone 必须为字符串");
          return;
        }
        phone = body["phone"].get<std::string>();
      }

      if (body.contains("status")) {
        if (!body["status"].is_number_integer()) {
          sendFail(res, 400, "status 必须为整数");
          return;
        }
        const int parsedStatus = body["status"].get<int>();
        if (parsedStatus != 0 && parsedStatus != 1) {
          sendFail(res, 400, "status 只能为 0 或 1");
          return;
        }
        status = parsedStatus;
      }

      if (body.contains("role_id")) {
        if (!body["role_id"].is_number_integer()) {
          sendFail(res, 400, "role_id 必须为整数");
          return;
        }
        roleId = body["role_id"].get<int>();
      }

      if (body.contains("password")) {
        if (!body["password"].is_string() ||
            body["password"].get<std::string>().empty()) {
          sendFail(res, 400, "password 必须为非空字符串");
          return;
        }
        password = body["password"].get<std::string>();
      }

      if (!realName.has_value() && !phone.has_value() && !status.has_value() &&
          !roleId.has_value() && !password.has_value()) {
        sendFail(res, 400, "至少提供一个可更新字段");
        return;
      }

      RoleDAO roleDao;
      if (roleId.has_value() && !roleDao.exists(*roleId)) {
        sendFail(res, 400, "角色不存在");
        return;
      }

      UserDAO userDao;
      if (!userDao.updateUser(userId, realName, phone, status, roleId,
                              password)) {
        sendFail(res, 404, "用户不存在");
        return;
      }

      auto updatedUser = userDao.findById(userId);
      if (!updatedUser.has_value()) {
        sendFail(res, 500, "用户更新成功但读取结果失败");
        return;
      }

      if (updatedUser->status != 1) {
        AuthSessionManager::revokeUserTokens(userId);
      }

      sendOk(res, userToJson(*updatedUser), "更新用户成功");
    } catch (const std::exception &e) {
      logServerError("updateUser", e);
      sendFail(res, 500, "更新用户失败，请稍后重试");
    }
  });

  svr.Delete(R"(/api/v1/users/(\d+))", [](const httplib::Request &req,
                                          httplib::Response &res) {
    auto currentUser = AuthSessionManager::requireUser(req, res, kAdminRoleIds);
    if (!currentUser.has_value()) {
      return;
    }

    try {
      const int userId = std::atoi(std::string(req.matches[1]).c_str());
      UserDAO userDao;
      if (!userDao.softDeleteUser(userId)) {
        sendFail(res, 404, "用户不存在");
        return;
      }

      AuthSessionManager::revokeUserTokens(userId);
      sendOk(res, json{{"id", userId}, {"status", 0}}, "删除用户成功");
    } catch (const std::exception &e) {
      logServerError("deleteUser", e);
      sendFail(res, 500, "删除用户失败，请稍后重试");
    }
  });
}
