#include "DiseaseController.h"

#include "../services/DiseaseService.h"
#include "../utils/AuthSessionManager.h"
#include "../utils/ResponseHelper.h"

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
  std::cerr << "[DiseaseController] " << context << ": " << e.what()
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

} // namespace

void DiseaseController::registerRoutes(httplib::Server &svr) {
  // GET /api/v1/diseases
  svr.Get("/api/v1/diseases",
          [](const httplib::Request &req, httplib::Response &res) {
            auto currentUser = AuthSessionManager::requireUser(req, res);
            if (!currentUser.has_value())
              return;
            try {
              DiseaseService svc;
              json data = svc.getDiseaseList();
              sendOk(res, data, "查询成功");
            } catch (const std::exception &e) {
              logServerError("getDiseaseList", e);
              sendFail(res, 500, "查询慢性病字典失败，请稍后重试");
            }
          });

  // GET /api/v1/residents/{id}/diseases
  svr.Get(R"(/api/v1/residents/(\d+)/diseases)", [](const httplib::Request &req,
                                                    httplib::Response &res) {
    auto currentUser = AuthSessionManager::requireUser(req, res);
    if (!currentUser.has_value())
      return;
    try {
      const int resident_id = std::atoi(std::string(req.matches[1]).c_str());

      DiseaseService svc;
      json data = svc.getResidentDiseases(resident_id);
      if (data.is_object() && data.contains("error")) {
        sendFail(res, data.value("not_found", false) ? 404 : 400,
                 data["error"].get<std::string>());
        return;
      }
      sendOk(res, data, "查询成功");
    } catch (const std::exception &e) {
      logServerError("getResidentDiseases", e);
      sendFail(res, 500, "查询居民慢性病失败，请稍后重试");
    }
  });

  // POST /api/v1/residents/{id}/diseases
  svr.Post(R"(/api/v1/residents/(\d+)/diseases)",
           [](const httplib::Request &req, httplib::Response &res) {
             auto currentUser = AuthSessionManager::requireUser(req, res);
             if (!currentUser.has_value())
               return;
             try {
               const int resident_id =
                   std::atoi(std::string(req.matches[1]).c_str());
               json body;
               if (!parseJsonBody(req, res, body))
                 return;

               DiseaseService svc;
               json data = svc.addResidentDisease(resident_id, body);
               if (data.is_object() && data.contains("error")) {
                 sendFail(res, data.value("not_found", false) ? 404 : 400,
                          data["error"].get<std::string>());
                 return;
               }
               sendOk(res, data, "关联慢性病成功");
             } catch (const std::exception &e) {
               logServerError("addResidentDisease", e);
               sendFail(res, 500, "关联慢性病失败，请稍后重试");
             }
           });
}
