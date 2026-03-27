/**
 * @file main.cpp
 * @brief 社区健康档案管理系统 — 后端服务器入口
 *
 * 职责：
 *  1. 读取数据库连接配置，初始化 DatabaseManager 单例
 *  2. 创建 cpp-httplib HTTP 服务器实例
 *  3. 注册全局 CORS 预检处理器
 *  4. 按模块注册各业务路由（由各 Controller 提供静态方法 registerRoutes）
 *  5. 在指定 HOST:PORT 上启动监听
 *
 * 编译方式（详见 CMakeLists.txt）：
 *   cd backend && cmake -B build && cmake --build build
 *   ./build/community_health_server
 */

// ── 第三方头文件（固定使用项目本地副本，便于 IDE 识别）─────────────
#include "include/httplib.h"
#include "include/json.hpp"

// ── 项目内部头文件 ────────────────────────────────────────────
#include "utils/DatabaseManager.h"
#include "utils/ResponseHelper.h"

// ── 各模块控制器（逐步取消注释，随模块开发推进）─────────────────
#include "controllers/AuthController.h"
#include "controllers/HealthController.h"
#include "controllers/ResidentController.h"
#include "controllers/UserController.h"

#include "controllers/DiseaseController.h"
#include "controllers/VisitController.h"

// ── 标准库 ───────────────────────────────────────────────────
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

using json = nlohmann::json;
namespace fs = std::filesystem;

// ============================================================
// 数据库连接配置辅助函数
// ============================================================

/**
 * @brief 读取环境变量，若不存在则返回默认值
 * @param key          环境变量名
 * @param default_val  默认值
 * @return 环境变量的值或默认值
 */
static std::string getEnv(const char *key, const std::string &default_val) {
  const char *val = std::getenv(key);
  return (val != nullptr) ? std::string(val) : default_val;
}

// ============================================================
// 数据库连接配置（优先读取环境变量，未设置时使用默认值）
// 推荐在启动前设置环境变量，例如：
//   export DB_HOST=127.0.0.1
//   export DB_PORT=3306
//   export DB_USER=ch_admin
//   export DB_PASS=your_password
//   export DB_NAME=community_health
// ============================================================
static const std::string DB_HOST = getEnv("DB_HOST", "127.0.0.1");
static const int DB_PORT = []() {
  const char *p = std::getenv("DB_PORT");
  return (p != nullptr) ? std::atoi(p) : 3306;
}();
static const std::string DB_USER = getEnv("DB_USER", "ch_admin");
static const std::string DB_PASS =
    getEnv("DB_PASS", ""); // 密码必须通过环境变量提供
static const std::string DB_NAME = getEnv("DB_NAME", "community_health");

// ============================================================
// 服务器监听配置
// ============================================================
static const std::string SERVER_HOST = "0.0.0.0";
static const int SERVER_PORT = 8080;

// ============================================================
// 注册全局中间件（CORS 预检）
// ============================================================
/**
 * @brief 为服务器实例注册 OPTIONS 预检路由，处理浏览器跨域请求
 * @param svr  cpp-httplib 服务器实例引用
 */
static void registerCorsHandler(httplib::Server &svr) {
  // OPTIONS 预检：浏览器在跨域 POST/PUT/DELETE 前会先发 OPTIONS 请求
  svr.Options(".*",
              [](const httplib::Request & /*req*/, httplib::Response &res) {
                ResponseHelper::setCorsHeaders(res);
                res.status = 204; // No Content
              });
}

// ============================================================
// 注册系统内置路由
// ============================================================
/**
 * @brief 注册与具体业务模块无关的基础路由（健康检查等）
 * @param svr  cpp-httplib 服务器实例引用
 */
static void registerSystemRoutes(httplib::Server &svr) {
  // ── GET /api/v1/health-check ──────────────────────────────
  // 用于运维 / 前端探测后端服务是否存活
  svr.Get("/api/v1/health-check", [](const httplib::Request & /*req*/,
                                     httplib::Response &res) {
    ResponseHelper::setCorsHeaders(res);
    ResponseHelper::ok(res, json{{"status", "ok"}, {"version", "1.0.0"}},
                       "服务运行正常");
  });
}

/**
 * @brief 解析前端静态目录的实际路径，兼容不同启动目录
 * @return 可用的 frontend 目录绝对路径；未找到则返回空字符串
 */
static std::string resolveFrontendMountPath() {
  // 1. 优先尝试挂载 Vue3 编译完成后的 dist 目录
  const std::vector<fs::path> vueCandidates = {
      fs::current_path() / "vue-frontend" / "dist",
      fs::current_path() / ".." / "vue-frontend" / "dist",
      fs::current_path() / ".." / ".." / "vue-frontend" / "dist",
  };

  for (const auto &candidate : vueCandidates) {
    std::error_code ec;
    const fs::path normalized = fs::weakly_canonical(candidate, ec);
    const fs::path finalPath = ec ? fs::absolute(candidate) : normalized;

    if (fs::exists(finalPath / "index.html")) {
      return finalPath.lexically_normal().string();
    }
  }

  // 2. 如果未找到 Vue3 dist (即未编译)，则回退尝试挂载旧版 frontend_old
  const std::vector<fs::path> oldCandidates = {
      fs::current_path() / "frontend_old",
      fs::current_path() / ".." / "frontend_old",
      fs::current_path() / ".." / ".." / "frontend_old",
  };

  for (const auto &candidate : oldCandidates) {
    std::error_code ec;
    const fs::path normalized = fs::weakly_canonical(candidate, ec);
    const fs::path finalPath = ec ? fs::absolute(candidate) : normalized;

    if (fs::exists(finalPath / "pages" / "login.html")) {
      return finalPath.lexically_normal().string();
    }
  }

  return "";
}

/**
 * @brief 注册前端入口路由，保证访问根路径时可直接进入登录页
 * @param svr cpp-httplib 服务器实例引用
 */
static void registerFrontendRoutes(httplib::Server &svr, bool legacyFrontend) {
  if (legacyFrontend) {
    svr.Get("/", [](const httplib::Request & /*req*/, httplib::Response &res) {
      res.set_redirect("/pages/login.html");
    });

    svr.Get("/index.html",
            [](const httplib::Request & /*req*/, httplib::Response &res) {
              res.set_redirect("/pages/login.html");
            });
    return;
  }

  svr.Get("/", [](const httplib::Request & /*req*/, httplib::Response &res) {
    res.set_redirect("/index.html");
  });
}

// ============================================================
// 注册各业务模块路由
// ============================================================
/**
 * @brief 汇总调用所有业务控制器的 registerRoutes 方法
 *
 * 每完成一个业务模块的开发，取消对应 Controller 的
 * #include 和 registerRoutes(svr) 调用即可接入路由。
 *
 * @param svr  cpp-httplib 服务器实例引用
 */
static void registerBusinessRoutes(httplib::Server &svr) {
  // 模块 2：用户认证与权限
  AuthController::registerRoutes(svr);
  UserController::registerRoutes(svr);

  // 模块 3：居民档案（含社区管理）
  ResidentController::registerRoutes(svr);

  // 模块 4：健康测量与预警
  HealthController::registerRoutes(svr);

  // 模块 5：慢性病管理
  DiseaseController::registerRoutes(svr);

  // 模块 6：随访管理
  VisitController::registerRoutes(svr);
}

// ============================================================
// main()
// ============================================================
int main() {
  // ── 1. 初始化数据库连接 ────────────────────────────────────
  try {
    DatabaseManager::getInstance().init(DB_HOST, DB_PORT, DB_USER, DB_PASS,
                                        DB_NAME);
  } catch (const std::exception &e) {
    std::cerr << "[错误] 数据库初始化失败: " << e.what() << std::endl;
    return EXIT_FAILURE;
  }

  // ── 2. 创建 HTTP 服务器实例 ────────────────────────────────
  httplib::Server svr;

  // ── 3. 注册 CORS 预检处理器 ────────────────────────────────
  registerCorsHandler(svr);

  // ── 4. 注册系统路由 ────────────────────────────────────────
  registerSystemRoutes(svr);

  // ── 5. 注册业务路由 ────────────────────────────────────────
  registerBusinessRoutes(svr);

  // ── 6. 挂载前端静态文件服务 ──────────────────────────────────
  const std::string frontendMountPath = resolveFrontendMountPath();
  const bool legacyFrontend =
      !frontendMountPath.empty() &&
      fs::exists(fs::path(frontendMountPath) / "pages" / "login.html");
  if (frontendMountPath.empty()) {
    std::cerr << "[警告] 未找到 frontend 静态目录，页面资源将无法访问。"
              << std::endl;
  } else if (!svr.set_mount_point("/", frontendMountPath.c_str())) {
    std::cerr << "[警告] frontend 静态目录挂载失败: " << frontendMountPath
              << std::endl;
  }

  // ── 7. 注册前端入口路由 ────────────────────────────────────
  registerFrontendRoutes(svr, legacyFrontend);

  // ── 8. 启动服务器 ─────────────────────────────────────────
  std::cout << "================================================" << std::endl;
  std::cout << " 社区健康档案管理系统 — 后端服务器" << std::endl;
  std::cout << " 监听地址: http://" << SERVER_HOST << ":" << SERVER_PORT
            << std::endl;
  std::cout << " API Base: http://localhost:" << SERVER_PORT << "/api/v1"
            << std::endl;
  std::cout << "================================================" << std::endl;

  if (!svr.listen(SERVER_HOST.c_str(), SERVER_PORT)) {
    std::cerr << "[错误] 服务器启动失败，端口 " << SERVER_PORT
              << " 可能已被占用。" << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
