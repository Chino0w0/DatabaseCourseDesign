#include "VisitController.h"

#include "../services/VisitService.h"
#include "../utils/AuthSessionManager.h"
#include "../utils/ResponseHelper.h"

#include <algorithm>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <string>

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
  std::cerr << "[VisitController] " << context << ": " << e.what() << std::endl;
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

void VisitController::registerRoutes(httplib::Server &svr) {
  // GET /api/v1/visits?resident_id=1
  svr.Get("/api/v1/visits",
          [](const httplib::Request &req, httplib::Response &res) {
            auto currentUser = AuthSessionManager::requireUser(req, res);
            if (!currentUser.has_value())
              return;
            try {
              if (!req.has_param("resident_id")) {
                sendFail(res, 400, "缺少必填参数 resident_id");
                return;
              }

              const int resident_id =
                  parsePositiveInt(req.get_param_value("resident_id"), -1);
              VisitService svc;
              json data = svc.getVisits(resident_id);
              if (data.is_object() && data.contains("error")) {
                sendFail(res, data.value("not_found", false) ? 404 : 400,
                         data["error"].get<std::string>());
                return;
              }
              sendOk(res, data, "查询成功");
            } catch (const std::exception &e) {
              logServerError("getVisits", e);
              sendFail(res, 500, "查询随访记录失败，请稍后重试");
            }
          });

  // POST /api/v1/visits
  svr.Post("/api/v1/visits",
           [](const httplib::Request &req, httplib::Response &res) {
             auto currentUser = AuthSessionManager::requireUser(req, res);
             if (!currentUser.has_value())
               return;
             try {
               json body;
               if (!parseJsonBody(req, res, body))
                 return;

               VisitService svc;
               json data = svc.createVisit(body);
               if (data.is_object() && data.contains("error")) {
                 sendFail(res, data.value("not_found", false) ? 404 : 400,
                          data["error"].get<std::string>());
                 return;
               }
               sendOk(res, data, "新增随访记录成功");
             } catch (const std::exception &e) {
               logServerError("createVisit", e);
               sendFail(res, 500, "新增随访记录失败，请稍后重试");
             }
           });
}
