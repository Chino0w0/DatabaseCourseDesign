#include "MyHealthController.h"

#include "../models/User.h"
#include "../services/MyHealthService.h"
#include "../utils/AuthSessionManager.h"
#include "../utils/ResponseHelper.h"

#include <algorithm>
#include <iostream>
#include <string>

namespace {

const std::vector<int> kResidentRoleIds{4, 5};

bool isResidentRole(const User &user) {
  return std::find(kResidentRoleIds.begin(), kResidentRoleIds.end(),
                   user.roleId) != kResidentRoleIds.end() ||
         user.roleName == "居民";
}

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
  std::cerr << "[MyHealthController] " << context << ": " << e.what()
            << std::endl;
}

bool ensureResidentUser(const User &user, httplib::Response &res) {
  if (!isResidentRole(user)) {
    sendFail(res, 403, "无权限访问该接口");
    return false;
  }
  return true;
}

bool ensureResidentBound(const User &user, httplib::Response &res) {
  if (user.residentId <= 0) {
    sendFail(res, 403, "当前居民账号未绑定居民档案");
    return false;
  }
  return true;
}

} // namespace

void MyHealthController::registerRoutes(httplib::Server &svr) {
  svr.Get("/api/v1/me/profile",
          [](const httplib::Request &req, httplib::Response &res) {
            auto currentUser = AuthSessionManager::requireUser(req, res);
            if (!currentUser.has_value())
              return;
            if (!ensureResidentUser(*currentUser, res) ||
                !ensureResidentBound(*currentUser, res))
              return;

            try {
              MyHealthService service;
              json data = service.getProfile(currentUser->residentId);
              if (data.is_object() && data.contains("error")) {
                sendFail(res, data.value("not_bound", false) ? 403 : 400,
                         data["error"].get<std::string>());
                return;
              }
              sendOk(res, data, "查询成功");
            } catch (const std::exception &e) {
              logServerError("getProfile", e);
              sendFail(res, 500, "查询本人健康档案失败，请稍后重试");
            }
          });

  svr.Get("/api/v1/me/measurements", [](const httplib::Request &req,
                                        httplib::Response &res) {
    auto currentUser = AuthSessionManager::requireUser(req, res);
    if (!currentUser.has_value())
      return;
    if (!ensureResidentUser(*currentUser, res) ||
        !ensureResidentBound(*currentUser, res))
      return;

    try {
      int limit = 50;
      if (req.has_param("limit")) {
        limit = std::max(
            1, std::min(200, std::atoi(req.get_param_value("limit").c_str())));
      }

      MyHealthService service;
      json data = service.getMeasurements(currentUser->residentId, limit);
      if (data.is_object() && data.contains("error")) {
        sendFail(res, data.value("not_bound", false) ? 403 : 400,
                 data["error"].get<std::string>());
        return;
      }
      sendOk(res, data, "查询成功");
    } catch (const std::exception &e) {
      logServerError("getMeasurements", e);
      sendFail(res, 500, "查询本人测量记录失败，请稍后重试");
    }
  });

  svr.Get("/api/v1/me/diseases",
          [](const httplib::Request &req, httplib::Response &res) {
            auto currentUser = AuthSessionManager::requireUser(req, res);
            if (!currentUser.has_value())
              return;
            if (!ensureResidentUser(*currentUser, res) ||
                !ensureResidentBound(*currentUser, res))
              return;

            try {
              MyHealthService service;
              json data = service.getDiseases(currentUser->residentId);
              if (data.is_object() && data.contains("error")) {
                sendFail(res, data.value("not_bound", false) ? 403 : 400,
                         data["error"].get<std::string>());
                return;
              }
              sendOk(res, data, "查询成功");
            } catch (const std::exception &e) {
              logServerError("getDiseases", e);
              sendFail(res, 500, "查询本人慢性病信息失败，请稍后重试");
            }
          });

  svr.Get("/api/v1/me/visits",
          [](const httplib::Request &req, httplib::Response &res) {
            auto currentUser = AuthSessionManager::requireUser(req, res);
            if (!currentUser.has_value())
              return;
            if (!ensureResidentUser(*currentUser, res) ||
                !ensureResidentBound(*currentUser, res))
              return;

            try {
              MyHealthService service;
              json data = service.getVisits(currentUser->residentId);
              if (data.is_object() && data.contains("error")) {
                sendFail(res, data.value("not_bound", false) ? 403 : 400,
                         data["error"].get<std::string>());
                return;
              }
              sendOk(res, data, "查询成功");
            } catch (const std::exception &e) {
              logServerError("getVisits", e);
              sendFail(res, 500, "查询本人随访记录失败，请稍后重试");
            }
          });
}
