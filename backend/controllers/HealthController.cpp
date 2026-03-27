#include "HealthController.h"

#include "../services/HealthService.h"
#include "../utils/ResponseHelper.h"

#include <algorithm>
#include <cstdlib>
#include <string>

namespace {

void sendOk(httplib::Response& res, const json& data, const std::string& msg = "操作成功") {
    ResponseHelper::setCorsHeaders(res);
    ResponseHelper::ok(res, data, msg);
}

void sendFail(httplib::Response& res, int code, const std::string& msg) {
    ResponseHelper::setCorsHeaders(res);
    ResponseHelper::fail(res, code, msg);
}

bool parseJsonBody(const httplib::Request& req, httplib::Response& res, json& body) {
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

int parsePositiveInt(const std::string& value, int fallback) {
    const int parsed = std::atoi(value.c_str());
    return parsed > 0 ? parsed : fallback;
}

} // namespace

void HealthController::registerRoutes(httplib::Server& svr) {
    // GET /api/v1/health/measurements?resident_id=1&limit=50
    svr.Get("/api/v1/health/measurements",
            [](const httplib::Request& req, httplib::Response& res) {
                try {
                    if (!req.has_param("resident_id")) {
                        sendFail(res, 400, "缺少必填参数 resident_id");
                        return;
                    }
                    const int resident_id = parsePositiveInt(req.get_param_value("resident_id"), -1);
                    const int limit = req.has_param("limit")
                                          ? std::min(parsePositiveInt(req.get_param_value("limit"), 50), 200)
                                          : 50;

                    HealthService svc;
                    json data = svc.getMeasurements(resident_id, limit);
                    if (data.is_object() && data.contains("error")) {
                        const bool not_found = data.value("not_found", false);
                        sendFail(res, not_found ? 404 : 400, data["error"].get<std::string>());
                        return;
                    }
                    sendOk(res, data, "查询成功");
                } catch (const std::exception& e) {
                    sendFail(res, 500, std::string("查询健康测量记录失败: ") + e.what());
                }
            });

    // POST /api/v1/health/measurements
    svr.Post("/api/v1/health/measurements",
             [](const httplib::Request& req, httplib::Response& res) {
                 try {
                     json body;
                     if (!parseJsonBody(req, res, body)) return;

                     HealthService svc;
                     json data = svc.createMeasurement(body);
                     if (data.is_object() && data.contains("error")) {
                         const bool not_found = data.value("not_found", false);
                         sendFail(res, not_found ? 404 : 400, data["error"].get<std::string>());
                         return;
                     }
                     sendOk(res, data, "录入测量数据成功");
                 } catch (const std::exception& e) {
                     sendFail(res, 500, std::string("录入测量数据失败: ") + e.what());
                 }
             });

    // GET /api/v1/health/records/{resident_id}
    svr.Get(R"(/api/v1/health/records/(\d+))",
            [](const httplib::Request& req, httplib::Response& res) {
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
                } catch (const std::exception& e) {
                    sendFail(res, 500, std::string("查询健康档案失败: ") + e.what());
                }
            });
}
