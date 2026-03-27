#include "../controllers/AuthController.h"

#include "../services/AuthService.h"
#include "../utils/ResponseHelper.h"

#include <exception>
#include <string>

namespace {

void sendOk(httplib::Response &res, const json &data, const std::string &msg) {
  ResponseHelper::setCorsHeaders(res);
  ResponseHelper::ok(res, data, msg);
}

void sendFail(httplib::Response &res, int code, const std::string &msg) {
  ResponseHelper::setCorsHeaders(res);
  ResponseHelper::fail(res, code, msg);
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

void AuthController::registerRoutes(httplib::Server &svr) {
  svr.Post("/api/v1/auth/login", [](const httplib::Request &req,
                                    httplib::Response &res) {
    try {
      json body;
      if (!parseJsonBody(req, res, body)) {
        return;
      }

      if (!body.contains("username") || !body["username"].is_string() ||
          !body.contains("password") || !body["password"].is_string()) {
        sendFail(res, 400, "用户名和密码不能为空");
        return;
      }

      const std::string username = body["username"].get<std::string>();
      const std::string password = body["password"].get<std::string>();

      if (username.empty() || password.empty()) {
        sendFail(res, 400, "用户名和密码不能为空");
        return;
      }

      AuthService service;
      AuthResult authResult = service.login(username, password);
      if (!authResult.success) {
        sendFail(res, authResult.code, authResult.msg);
        return;
      }

      sendOk(res,
             json{{"user_id", authResult.user.id},
                  {"username", authResult.user.username},
                  {"role", authResult.user.roleName},
                  {"token", authResult.token}},
             authResult.msg);
    } catch (const std::exception &e) {
      sendFail(res, 500, std::string("登录失败: ") + e.what());
    }
  });
}
