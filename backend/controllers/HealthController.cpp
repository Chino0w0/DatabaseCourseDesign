#include "HealthController.h"

#include "../services/HealthService.h"
#include "../utils/AuthSessionManager.h"
#include "../utils/ResponseHelper.h"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <string>

namespace {

const std::vector<int> kHealthReadRoleIds{1, 2, 3};
const std::vector<int> kHealthWriteRoleIds{1, 2, 3};
const std::vector<int> kHealthRecordWriteRoleIds{1, 2};
const std::vector<int> kHealthWarningReadRoleIds{1, 2, 3};

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
  std::cerr << "[HealthController] " << context << ": " << e.what()
            << std::endl;
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

int parsePositiveInt(const std::string &value, int fallback) {
  const int parsed = std::atoi(value.c_str());
  return parsed > 0 ? parsed : fallback;
}

} // namespace

void HealthController::registerRoutes(httplib::Server &svr) {
  // GET /api/v1/health/measurements?resident_id=1&limit=50
  svr.Get("/api/v1/health/measurements", [](const httplib::Request &req,
                                            httplib::Response &res) {
    auto currentUser =
        AuthSessionManager::requireUser(req, res, kHealthReadRoleIds);
    if (!currentUser.has_value()) {
      return;
    }

    try {
      if (!req.has_param("resident_id")) {
        sendFail(res, 400, "缺少必填参数 resident_id");
        return;
      }
      const int resident_id =
          parsePositiveInt(req.get_param_value("resident_id"), -1);
      const int limit =
          req.has_param("limit")
              ? std::min(parsePositiveInt(req.get_param_value("limit"), 50),
                         200)
              : 50;

      HealthService svc;
      json data = svc.getMeasurements(resident_id, limit);
      if (data.is_object() && data.contains("error")) {
        const bool not_found = data.value("not_found", false);
        sendFail(res, not_found ? 404 : 400, data["error"].get<std::string>());
        return;
      }
      sendOk(res, data, "查询成功");
    } catch (const std::exception &e) {
      logServerError("getMeasurements", e);
      sendFail(res, 500, "查询健康测量记录失败，请稍后重试");
    }
  });

  // POST /api/v1/health/measurements
  svr.Post("/api/v1/health/measurements", [](const httplib::Request &req,
                                             httplib::Response &res) {
    auto currentUser =
        AuthSessionManager::requireUser(req, res, kHealthWriteRoleIds);
    if (!currentUser.has_value()) {
      return;
    }

    try {
      json body;
      if (!parseJsonBody(req, res, body))
        return;

      HealthService svc;
      json data = svc.createMeasurement(body);
      if (data.is_object() && data.contains("error")) {
        const bool not_found = data.value("not_found", false);
        sendFail(res, not_found ? 404 : 400, data["error"].get<std::string>());
        return;
      }
      sendOk(res, data, "录入测量数据成功");
    } catch (const std::exception &e) {
      logServerError("createMeasurement", e);
      sendFail(res, 500, "录入测量数据失败，请稍后重试");
    }
  });

  // PUT /api/v1/health/measurements/{id}
  svr.Put(R"(/api/v1/health/measurements/(\d+))",
          [](const httplib::Request &req, httplib::Response &res) {
            auto currentUser =
                AuthSessionManager::requireUser(req, res, kHealthWriteRoleIds);
            if (!currentUser.has_value()) {
              return;
            }

            try {
              const int measurement_id =
                  std::atoi(std::string(req.matches[1]).c_str());
              json body;
              if (!parseJsonBody(req, res, body))
                return;

              HealthService svc;
              json data = svc.updateMeasurement(measurement_id, body);
              if (data.is_object() && data.contains("error")) {
                const bool not_found = data.value("not_found", false);
                sendFail(res, not_found ? 404 : 400,
                         data["error"].get<std::string>());
                return;
              }
              sendOk(res, data, "更新测量数据成功");
            } catch (const std::exception &e) {
              logServerError("updateMeasurement", e);
              sendFail(res, 500, "更新测量数据失败，请稍后重试");
            }
          });

  // GET /api/v1/health/records/{resident_id}
  svr.Get(R"(/api/v1/health/records/(\d+))", [](const httplib::Request &req,
                                                httplib::Response &res) {
    auto currentUser =
        AuthSessionManager::requireUser(req, res, kHealthReadRoleIds);
    if (!currentUser.has_value()) {
      return;
    }

    try {
      const int resident_id = std::atoi(std::string(req.matches[1]).c_str());
      HealthService svc;
      json data = svc.getHealthRecord(resident_id);
      if (data.is_object() && data.contains("error")) {
        const bool not_found = data.value("not_found", false);
        sendFail(res, not_found ? 404 : 400, data["error"].get<std::string>());
        return;
      }
      if (data.is_null()) {
        sendFail(res, 404, "健康档案不存在");
        return;
      }
      sendOk(res, data, "查询成功");
    } catch (const std::exception &e) {
      logServerError("getHealthRecord", e);
      sendFail(res, 500, "查询健康档案失败，请稍后重试");
    }
  });

  // PUT /api/v1/health/records/{resident_id}
  svr.Put(R"(/api/v1/health/records/(\d+))", [](const httplib::Request &req,
                                                httplib::Response &res) {
    auto currentUser =
        AuthSessionManager::requireUser(req, res, kHealthRecordWriteRoleIds);
    if (!currentUser.has_value()) {
      return;
    }

    try {
      const int resident_id = std::atoi(std::string(req.matches[1]).c_str());
      json body;
      if (!parseJsonBody(req, res, body))
        return;

      HealthService svc;
      json data = svc.updateHealthRecord(resident_id, body);
      if (data.is_object() && data.contains("error")) {
        const bool not_found = data.value("not_found", false);
        sendFail(res, not_found ? 404 : 400, data["error"].get<std::string>());
        return;
      }
      sendOk(res, data, "更新健康档案成功");
    } catch (const std::exception &e) {
      logServerError("updateHealthRecord", e);
      sendFail(res, 500, "更新健康档案失败，请稍后重试");
    }
  });

  // GET /api/v1/health/warnings/count
  svr.Get("/api/v1/health/warnings/count", [](const httplib::Request &req,
                                              httplib::Response &res) {
    auto currentUser =
        AuthSessionManager::requireUser(req, res, kHealthWarningReadRoleIds);
    if (!currentUser.has_value()) {
      return;
    }

    try {
      HealthService svc;
      json data = svc.getWarningCount();
      sendOk(res, data, "查询成功");
    } catch (const std::exception &e) {
      logServerError("getWarningCount", e);
      sendFail(res, 500, "查询异常预警统计失败，请稍后重试");
    }
  });
}
