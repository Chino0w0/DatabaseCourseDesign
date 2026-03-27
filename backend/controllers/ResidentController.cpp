#include "ResidentController.h"

#include "../services/ResidentService.h"
#include "../utils/AuthSessionManager.h"
#include "../utils/ResponseHelper.h"

#include <stdexcept>
#include <string>

namespace {
// 安全解析正整数参数：仅接受 >0 的值；非法、溢出或非正数时回退默认值。
int parsePositiveInt(const std::string &value, int fallback) {
  try {
    const int parsed = std::stoi(value);
    return parsed > 0 ? parsed : fallback;
  } catch (const std::invalid_argument &) {
    return fallback;
  } catch (const std::out_of_range &) {
    return fallback;
  }
}
} // namespace

// ============================================================
// 注册所有居民档案及社区路由
// ============================================================

void ResidentController::registerRoutes(httplib::Server &svr) {

  // ──────────────────────────────────────────────────────────
  // GET /api/v1/communities — 获取社区列表
  // ──────────────────────────────────────────────────────────
  svr.Get("/api/v1/communities",
          [](const httplib::Request &req, httplib::Response &res) {
            auto currentUser = AuthSessionManager::requireUser(req, res);
            if (!currentUser.has_value()) {
              return;
            }
            ResponseHelper::setCorsHeaders(res);
            try {
              ResidentService svc;
              json data = svc.getCommunityList();
              ResponseHelper::ok(res, data, "查询成功");
            } catch (const std::exception &e) {
              ResponseHelper::fail(res, 500,
                                   std::string("服务器内部错误: ") + e.what());
            }
          });

  // ──────────────────────────────────────────────────────────
  // POST /api/v1/communities — 新增社区
  // ──────────────────────────────────────────────────────────
  svr.Post("/api/v1/communities", [](const httplib::Request &req,
                                     httplib::Response &res) {
    auto currentUser = AuthSessionManager::requireUser(req, res);
    if (!currentUser.has_value()) {
      return;
    }
    ResponseHelper::setCorsHeaders(res);
    try {
      json body = json::parse(req.body);
      ResidentService svc;
      json result = svc.createCommunity(body);

      if (result.contains("error")) {
        ResponseHelper::fail(res, 400, result["error"].get<std::string>());
        return;
      }
      ResponseHelper::ok(res, result, "新增社区成功");
    } catch (const json::parse_error &) {
      ResponseHelper::fail(res, 400, "请求体不是合法的 JSON");
    } catch (const std::exception &e) {
      ResponseHelper::fail(res, 500,
                           std::string("服务器内部错误: ") + e.what());
    }
  });

  // ──────────────────────────────────────────────────────────
  // GET /api/v1/residents — 居民列表（分页 + 模糊搜索 + 社区过滤）
  // 查询参数：page(默认1)、size(默认10)、keyword、community_id
  // ──────────────────────────────────────────────────────────
  svr.Get("/api/v1/residents", [](const httplib::Request &req,
                                  httplib::Response &res) {
    auto currentUser = AuthSessionManager::requireUser(req, res);
    if (!currentUser.has_value()) {
      return;
    }
    ResponseHelper::setCorsHeaders(res);
    try {
      int page = 1, size = 10, community_id = 0;
      std::string keyword;

      if (req.has_param("page"))
        page = parsePositiveInt(req.get_param_value("page"), 1);
      if (req.has_param("size"))
        size = parsePositiveInt(req.get_param_value("size"), 10);
      if (req.has_param("keyword"))
        keyword = req.get_param_value("keyword");
      if (req.has_param("community_id"))
        community_id = parsePositiveInt(req.get_param_value("community_id"), 0);

      ResidentService svc;
      json data = svc.getResidentList(page, size, keyword, community_id);
      ResponseHelper::ok(res, data, "查询成功");
    } catch (const std::exception &e) {
      ResponseHelper::fail(res, 500,
                           std::string("服务器内部错误: ") + e.what());
    }
  });

  // ──────────────────────────────────────────────────────────
  // GET /api/v1/residents/:id — 居民详情
  // ──────────────────────────────────────────────────────────
  svr.Get(R"(/api/v1/residents/(\d+))",
          [](const httplib::Request &req, httplib::Response &res) {
            auto currentUser = AuthSessionManager::requireUser(req, res);
            if (!currentUser.has_value()) {
              return;
            }
            ResponseHelper::setCorsHeaders(res);
            try {
              int id = std::stoi(req.matches[1]);
              ResidentService svc;
              json data = svc.getResidentDetail(id);

              if (data.is_null()) {
                ResponseHelper::fail(res, 404, "居民不存在");
                return;
              }
              ResponseHelper::ok(res, data, "查询成功");
            } catch (const std::exception &e) {
              ResponseHelper::fail(res, 500,
                                   std::string("服务器内部错误: ") + e.what());
            }
          });

  // ──────────────────────────────────────────────────────────
  // POST /api/v1/residents — 新增居民
  // ──────────────────────────────────────────────────────────
  svr.Post("/api/v1/residents", [](const httplib::Request &req,
                                   httplib::Response &res) {
    auto currentUser = AuthSessionManager::requireUser(req, res);
    if (!currentUser.has_value()) {
      return;
    }
    ResponseHelper::setCorsHeaders(res);
    try {
      json body = json::parse(req.body);
      ResidentService svc;
      json result = svc.createResident(body);

      if (result.contains("error")) {
        ResponseHelper::fail(res, 400, result["error"].get<std::string>());
        return;
      }
      ResponseHelper::ok(res, result, "新增居民成功");
    } catch (const json::parse_error &) {
      ResponseHelper::fail(res, 400, "请求体不是合法的 JSON");
    } catch (const std::exception &e) {
      ResponseHelper::fail(res, 500,
                           std::string("服务器内部错误: ") + e.what());
    }
  });

  // ──────────────────────────────────────────────────────────
  // PUT /api/v1/residents/:id — 更新居民信息
  // ──────────────────────────────────────────────────────────
  svr.Put(R"(/api/v1/residents/(\d+))", [](const httplib::Request &req,
                                           httplib::Response &res) {
    auto currentUser = AuthSessionManager::requireUser(req, res);
    if (!currentUser.has_value()) {
      return;
    }
    ResponseHelper::setCorsHeaders(res);
    try {
      int id = std::stoi(req.matches[1]);
      json body = json::parse(req.body);
      ResidentService svc;
      json result = svc.updateResident(id, body);

      if (result.contains("error")) {
        // 区分 404（不存在）与 400（参数错误）
        bool not_found = result.value("not_found", false);
        int code = not_found ? 404 : 400;
        ResponseHelper::fail(res, code, result["error"].get<std::string>());
        return;
      }
      ResponseHelper::ok(res, nullptr, "更新居民信息成功");
    } catch (const json::parse_error &) {
      ResponseHelper::fail(res, 400, "请求体不是合法的 JSON");
    } catch (const std::exception &e) {
      ResponseHelper::fail(res, 500,
                           std::string("服务器内部错误: ") + e.what());
    }
  });

  // ──────────────────────────────────────────────────────────
  // DELETE /api/v1/residents/:id — 删除居民
  // ──────────────────────────────────────────────────────────
  svr.Delete(R"(/api/v1/residents/(\d+))",
             [](const httplib::Request &req, httplib::Response &res) {
               auto currentUser = AuthSessionManager::requireUser(req, res);
               if (!currentUser.has_value()) {
                 return;
               }
               ResponseHelper::setCorsHeaders(res);
               try {
                 int id = std::stoi(req.matches[1]);
                 ResidentService svc;
                 bool ok = svc.deleteResident(id);

                 if (!ok) {
                   ResponseHelper::fail(res, 404, "居民不存在");
                   return;
                 }
                 ResponseHelper::ok(res, nullptr, "删除居民成功");
               } catch (const std::exception &e) {
                 ResponseHelper::fail(
                     res, 500, std::string("服务器内部错误: ") + e.what());
               }
             });
}
