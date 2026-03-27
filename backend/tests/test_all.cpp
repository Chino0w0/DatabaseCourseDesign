/**
 * @file test_all.cpp
 * @brief 后端基础架构（模块1）完整测试套件
 *
 * 覆盖范围：
 *  - ResponseHelper 单元测试（JSON 结构、HTTP 状态码映射、CORS 头）
 *  - DatabaseManager 集成测试（单例、未初始化异常、错误凭据、连接、
 *    query/execute/escape/lastInsertId）
 *  - 端到端测试（启动 HTTP 服务器线程，调用 /api/v1/health-check）
 *
 * 编译方式（由 CMakeLists.txt 中的 community_health_tests 目标负责）：
 *   cd backend && cmake -B build && cmake --build build --target community_health_tests
 *   ./build/community_health_tests
 *
 * 需要 MySQL 运行时环境变量（集成测试）：
 *   TEST_DB_HOST  (默认 127.0.0.1)
 *   TEST_DB_PORT  (默认 3306)
 *   TEST_DB_USER  (默认 ch_admin)
 *   TEST_DB_PASS  （必须显式设置，无默认值；对应 sql/permissions.sql 中 ch_admin 的密码）
 *   TEST_DB_NAME  (默认 community_health)
 *
 * 示例：
 *   export TEST_DB_PASS=Admin@2026!
 *   ./build/community_health_tests
 */

// ── 项目头文件 ────────────────────────────────────────────────
#include "../utils/ResponseHelper.h"
#include "../utils/DatabaseManager.h"

// ── 标准库 ───────────────────────────────────────────────────
#include <iostream>
#include <string>
#include <sstream>
#include <stdexcept>
#include <thread>
#include <chrono>
#include <cstdlib>
#include <atomic>

// ============================================================
// 轻量级测试框架（无外部依赖）
// ============================================================
namespace TestRunner {
    static int total   = 0;  ///< 测试总数
    static int passed  = 0;  ///< 通过数
    static int failed  = 0;  ///< 失败数

    /// 记录一条通过的断言
    inline void pass(const std::string& name) {
        ++total; ++passed;
        std::cout << "  [PASS] " << name << std::endl;
    }

    /// 记录一条失败的断言
    inline void fail(const std::string& name, const std::string& detail) {
        ++total; ++failed;
        std::cout << "  [FAIL] " << name << "\n         " << detail << std::endl;
    }

    /// 打印最终统计信息，返回是否全部通过
    inline bool summary() {
        std::cout << "\n========================================\n";
        std::cout << "测试结果: " << passed << "/" << total
                  << " 通过" << (failed > 0 ? "，" + std::to_string(failed) + " 失败" : "")
                  << std::endl;
        std::cout << "========================================\n";
        return (failed == 0);
    }
} // namespace TestRunner

// ── 断言宏 ───────────────────────────────────────────────────

/** 检查表达式为 true */
#define EXPECT_TRUE(name, expr)                                    \
    do {                                                           \
        if ((expr)) {                                              \
            TestRunner::pass(name);                                \
        } else {                                                   \
            TestRunner::fail(name, "期望 true，但结果为 false");   \
        }                                                          \
    } while(0)

/** 检查两个值相等 */
#define EXPECT_EQ(name, actual, expected)                          \
    do {                                                           \
        auto _a = (actual); auto _e = (expected);                  \
        if (_a == _e) {                                            \
            TestRunner::pass(name);                                \
        } else {                                                   \
            std::ostringstream _oss;                               \
            _oss << "期望 [" << _e << "] 实际 [" << _a << "]";   \
            TestRunner::fail(name, _oss.str());                    \
        }                                                          \
    } while(0)

/** 检查字符串包含子串 */
#define EXPECT_CONTAINS(name, str, substr)                         \
    do {                                                           \
        std::string _s(str), _sub(substr);                         \
        if (_s.find(_sub) != std::string::npos) {                  \
            TestRunner::pass(name);                                \
        } else {                                                   \
            TestRunner::fail(name, "\"" + _s + "\" 中未找到子串 \"" + _sub + "\""); \
        }                                                          \
    } while(0)

/** 检查调用 fn() 会抛出异常 */
#define EXPECT_THROW(name, fn)                                     \
    do {                                                           \
        bool _threw = false;                                       \
        try { fn(); } catch (...) { _threw = true; }               \
        if (_threw) {                                              \
            TestRunner::pass(name);                                \
        } else {                                                   \
            TestRunner::fail(name, "期望抛出异常，但未抛出");       \
        }                                                          \
    } while(0)

/** 检查调用 fn() 不会抛出异常 */
#define EXPECT_NO_THROW(name, fn)                                  \
    do {                                                           \
        try {                                                      \
            fn();                                                  \
            TestRunner::pass(name);                                \
        } catch (const std::exception& _ex) {                      \
            TestRunner::fail(name, std::string("意外异常: ") + _ex.what()); \
        }                                                          \
    } while(0)

// ============================================================
// 辅助：读取测试环境变量
// ============================================================
static std::string testEnv(const char* key, const std::string& def) {
    const char* v = std::getenv(key);
    return v ? std::string(v) : def;
}

static int testEnvInt(const char* key, int def) {
    const char* v = std::getenv(key);
    return v ? std::atoi(v) : def;
}

// ============================================================
// 第一部分：ResponseHelper 单元测试（无 MySQL 依赖）
// ============================================================

void testResponseHelperSuccess() {
    std::cout << "\n[ResponseHelper] success() 构造测试\n";

    // 1. 默认调用：code=200，msg="操作成功"，data=null
    {
        json j = ResponseHelper::success();
        EXPECT_EQ("success() code 字段为 200",    j["code"].get<int>(), 200);
        EXPECT_EQ("success() msg 字段为 '操作成功'", j["msg"].get<std::string>(), std::string("操作成功"));
        EXPECT_TRUE("success() data 字段为 null",  j["data"].is_null());
    }

    // 2. 带业务数据
    {
        json data = {{"id", 1}, {"name", "张三"}};
        json j = ResponseHelper::success(data);
        EXPECT_EQ("success(data) code 字段为 200", j["code"].get<int>(), 200);
        EXPECT_EQ("success(data) data.id 字段为 1", j["data"]["id"].get<int>(), 1);
        EXPECT_EQ("success(data) data.name 字段正确", j["data"]["name"].get<std::string>(), std::string("张三"));
    }

    // 3. 自定义 msg
    {
        json j = ResponseHelper::success(nullptr, "登录成功");
        EXPECT_EQ("success(data, msg) msg 字段为 '登录成功'", j["msg"].get<std::string>(), std::string("登录成功"));
    }

    // 4. 嵌套对象
    {
        json data = {{"total", 100}, {"list", json::array({1, 2, 3})}};
        json j = ResponseHelper::success(data, "查询成功");
        EXPECT_EQ("success() 嵌套 total=100",    j["data"]["total"].get<int>(), 100);
        EXPECT_EQ("success() list 长度为 3",     (int)j["data"]["list"].size(), 3);
    }
}

void testResponseHelperError() {
    std::cout << "\n[ResponseHelper] error() 构造测试\n";

    // 1. 400 参数错误
    {
        json j = ResponseHelper::error(400, "参数错误");
        EXPECT_EQ("error(400) code=400", j["code"].get<int>(), 400);
        EXPECT_EQ("error(400) msg 正确", j["msg"].get<std::string>(), std::string("参数错误"));
        EXPECT_TRUE("error(400) data=null", j["data"].is_null());
    }

    // 2. 401 未认证
    {
        json j = ResponseHelper::error(401, "用户名或密码错误");
        EXPECT_EQ("error(401) code=401", j["code"].get<int>(), 401);
        EXPECT_TRUE("error(401) data=null", j["data"].is_null());
    }

    // 3. 500 服务器错误
    {
        json j = ResponseHelper::error(500, "服务器内部错误");
        EXPECT_EQ("error(500) code=500", j["code"].get<int>(), 500);
    }
}

void testResponseHelperSendJson() {
    std::cout << "\n[ResponseHelper] sendJson() / ok() / fail() 测试\n";

    // ok() → HTTP 200，body 含 code:200
    {
        httplib::Response res;
        ResponseHelper::ok(res);
        EXPECT_EQ("ok() HTTP status=200", res.status, 200);
        json body = json::parse(res.body);
        EXPECT_EQ("ok() body.code=200", body["code"].get<int>(), 200);
        EXPECT_EQ("ok() body.msg='操作成功'", body["msg"].get<std::string>(), std::string("操作成功"));
        EXPECT_TRUE("ok() body.data=null", body["data"].is_null());
    }

    // ok(res, data, msg)
    {
        httplib::Response res;
        json data = {{"user_id", 42}};
        ResponseHelper::ok(res, data, "登录成功");
        json body = json::parse(res.body);
        EXPECT_EQ("ok(data,msg) body.msg='登录成功'", body["msg"].get<std::string>(), std::string("登录成功"));
        EXPECT_EQ("ok(data,msg) data.user_id=42", body["data"]["user_id"].get<int>(), 42);
    }

    // fail(400) → HTTP 400
    {
        httplib::Response res;
        ResponseHelper::fail(res, 400, "缺少必填字段");
        EXPECT_EQ("fail(400) HTTP status=400", res.status, 400);
        json body = json::parse(res.body);
        EXPECT_EQ("fail(400) body.code=400", body["code"].get<int>(), 400);
        EXPECT_EQ("fail(400) body.msg 正确", body["msg"].get<std::string>(), std::string("缺少必填字段"));
    }

    // fail(401) → HTTP 401
    {
        httplib::Response res;
        ResponseHelper::fail(res, 401, "未认证");
        EXPECT_EQ("fail(401) HTTP status=401", res.status, 401);
    }

    // fail(404) → HTTP 404
    {
        httplib::Response res;
        ResponseHelper::fail(res, 404, "资源不存在");
        EXPECT_EQ("fail(404) HTTP status=404", res.status, 404);
    }

    // fail(500) → HTTP 500
    {
        httplib::Response res;
        ResponseHelper::fail(res, 500, "服务器内部错误");
        EXPECT_EQ("fail(500) HTTP status=500", res.status, 500);
    }

    // fail(503) → HTTP 500（所有 >=500 的业务码映射到 HTTP 500）
    {
        httplib::Response res;
        ResponseHelper::fail(res, 503, "服务暂时不可用");
        EXPECT_EQ("fail(503) HTTP status=500（>=500 均映射到 500）", res.status, 500);
        json body = json::parse(res.body);
        // 业务 code 字段保持原始 503
        EXPECT_EQ("fail(503) body.code=503", body["code"].get<int>(), 503);
    }

    // Content-Type 必须包含 application/json
    {
        httplib::Response res;
        ResponseHelper::ok(res);
        EXPECT_CONTAINS("ok() Content-Type 含 application/json",
                        res.get_header_value("Content-Type"), "application/json");
    }
}

void testResponseHelperCors() {
    std::cout << "\n[ResponseHelper] setCorsHeaders() CORS 头测试\n";

    httplib::Response res;
    ResponseHelper::setCorsHeaders(res);

    EXPECT_EQ("CORS Allow-Origin = *",
              res.get_header_value("Access-Control-Allow-Origin"),
              std::string("*"));

    EXPECT_CONTAINS("CORS Allow-Methods 含 GET",
                    res.get_header_value("Access-Control-Allow-Methods"), "GET");

    EXPECT_CONTAINS("CORS Allow-Methods 含 POST",
                    res.get_header_value("Access-Control-Allow-Methods"), "POST");

    EXPECT_CONTAINS("CORS Allow-Methods 含 DELETE",
                    res.get_header_value("Access-Control-Allow-Methods"), "DELETE");

    EXPECT_CONTAINS("CORS Allow-Headers 含 Content-Type",
                    res.get_header_value("Access-Control-Allow-Headers"), "Content-Type");

    EXPECT_CONTAINS("CORS Allow-Headers 含 Authorization",
                    res.get_header_value("Access-Control-Allow-Headers"), "Authorization");

    EXPECT_EQ("CORS Max-Age = 86400",
              res.get_header_value("Access-Control-Max-Age"),
              std::string("86400"));
}

// ============================================================
// 第二部分：DatabaseManager 集成测试（需要 MySQL）
// ============================================================

/**
 * @brief 测试 DatabaseManager 的全部功能
 *
 * 注意：单例的 initialized_ 状态在进程内保持。
 * 测试顺序必须为：
 *   1. 未初始化时 getConnection 抛出
 *   2. 错误凭据 init 抛出
 *   3. 正确凭据 init 成功
 *   4. init 再次调用幂等
 *   5. query / execute / escape / lastInsertId 功能测试
 */
void testDatabaseManagerIntegration() {
    const std::string host    = testEnv("TEST_DB_HOST", "127.0.0.1");
    const int         port    = testEnvInt("TEST_DB_PORT", 3306);
    const std::string user    = testEnv("TEST_DB_USER", "ch_admin");
    const std::string pass    = testEnv("TEST_DB_PASS", "");
    const std::string db      = testEnv("TEST_DB_NAME", "community_health");

    std::cout << "\n[DatabaseManager] 集成测试（" << user << "@" << host
              << ":" << port << "/" << db << "）\n";

    // 若未提供密码则跳过集成测试，避免使用硬编码凭据
    if (pass.empty()) {
        std::cout << "  [SKIP] TEST_DB_PASS 未设置，跳过数据库集成测试。\n"
                  << "         请设置环境变量后重新运行（密码见 sql/permissions.sql）。\n";
        return;
    }

    // ── 1. 未初始化时 getConnection() 抛出 ────────────────────
    EXPECT_THROW("未初始化时 getConnection() 应抛出 runtime_error",
                 []() { DatabaseManager::getInstance().getConnection(); });

    // ── 2. 错误凭据 init() 抛出 ───────────────────────────────
    EXPECT_THROW("错误凭据 init() 应抛出 runtime_error",
                 [&]() {
                     DatabaseManager::getInstance().init(
                         host, port, user, "WRONG_PASSWORD_XYZ", db);
                 });

    // ── 3. 正确凭据 init() 成功 ────────────────────────────────
    EXPECT_NO_THROW("正确凭据 init() 不应抛出",
                    [&]() {
                        DatabaseManager::getInstance().init(host, port, user, pass, db);
                    });

    // ── 4. 再次调用 init() 应幂等（不重连） ────────────────────
    EXPECT_NO_THROW("再次调用 init() 应幂等（无异常）",
                    [&]() {
                        DatabaseManager::getInstance().init(host, port, user, pass, db);
                    });

    // ── 5. 单例一致性 ─────────────────────────────────────────
    EXPECT_TRUE("getInstance() 返回同一引用",
                &DatabaseManager::getInstance() == &DatabaseManager::getInstance());

    // ── 6. getConnection() 返回有效句柄 ───────────────────────
    EXPECT_NO_THROW("getConnection() 不应抛出",
                    []() {
                        MYSQL* c = DatabaseManager::getInstance().getConnection();
                        if (!c) throw std::runtime_error("conn 为 nullptr");
                    });

    // ── 7. query() — SELECT 1 ────────────────────────────────
    {
        bool ok = false;
        try {
            MYSQL_RES* res = DatabaseManager::getInstance().query("SELECT 1 AS n");
            if (res) {
                MYSQL_ROW row = mysql_fetch_row(res);
                if (row && row[0] && std::string(row[0]) == "1") {
                    ok = true;
                }
                mysql_free_result(res);
            }
        } catch (...) {}
        EXPECT_TRUE("query('SELECT 1') 返回正确结果", ok);
    }

    // ── 8. query() — 查询 roles 表 ──────────────────────────
    {
        bool ok = false;
        try {
            MYSQL_RES* res = DatabaseManager::getInstance().query(
                "SELECT COUNT(*) FROM roles");
            if (res) {
                MYSQL_ROW row = mysql_fetch_row(res);
                // init.sql 插入了 3 条角色：管理员、医生、护士
                // 用 atoi 避免 stoi 在非数字串时抛出异常
                ok = row && row[0] && std::strlen(row[0]) > 0
                     && std::atoi(row[0]) >= 3;
                mysql_free_result(res);
            }
        } catch (...) {}
        EXPECT_TRUE("query(roles) 至少存在 3 条角色记录", ok);
    }

    // ── 9. query() — 无效 SQL 应抛出 ────────────────────────
    EXPECT_THROW("query(无效SQL) 应抛出 runtime_error",
                 []() { DatabaseManager::getInstance().query("SELECT * FROM 不存在的表_xyz"); });

    // ── 10. execute() — INSERT / DELETE ─────────────────────
    {
        // 先清理，再插入临时社区，再删除
        bool ok = false;
        try {
            // 使用足够特殊的名称避免与正式数据冲突
            std::string escape_name = DatabaseManager::getInstance().escape("测试社区_unittest");
            DatabaseManager::getInstance().execute(
                "INSERT INTO communities (name, address) VALUES ('"
                + escape_name + "', 'unit-test-addr')");
            unsigned long long new_id = DatabaseManager::getInstance().lastInsertId();
            if (new_id > 0) {
                DatabaseManager::getInstance().execute(
                    "DELETE FROM communities WHERE id = " + std::to_string(new_id));
                ok = true;
            }
        } catch (const std::exception& e) {
            std::cerr << "         execute() 异常: " << e.what() << std::endl;
        }
        EXPECT_TRUE("execute(INSERT/DELETE) 完整流程成功", ok);
    }

    // ── 11. lastInsertId() — 自增 ID 大于 0 ─────────────────
    {
        bool ok = false;
        try {
            std::string name = DatabaseManager::getInstance().escape("自增测试社区_xyz");
            DatabaseManager::getInstance().execute(
                "INSERT INTO communities (name) VALUES ('" + name + "')");
            unsigned long long id = DatabaseManager::getInstance().lastInsertId();
            if (id > 0) {
                // 清理
                DatabaseManager::getInstance().execute(
                    "DELETE FROM communities WHERE id = " + std::to_string(id));
                ok = true;
            }
        } catch (...) {}
        EXPECT_TRUE("lastInsertId() 返回有效的自增 ID (>0)", ok);
    }

    // ── 12. escape() — SQL 注入字符转义 ──────────────────────
    {
        std::string raw      = "O'Brien; DROP TABLE users; --";
        std::string escaped  = DatabaseManager::getInstance().escape(raw);
        // 结果中不应包含未转义的单引号（原始的 ' 应变为 \'）
        // 简单检查：转义后长度 > 原始长度（说明有字符被转义）
        EXPECT_TRUE("escape() 对含单引号字符串的长度增加",
                    escaped.size() > raw.size());
        // 不应出现连续的未转义 '
        EXPECT_TRUE("escape() 结果中 ' 前有反斜杠",
                    escaped.find("\\'") != std::string::npos);
    }

    // ── 13. escape() — 空字符串 ──────────────────────────────
    {
        std::string escaped = DatabaseManager::getInstance().escape("");
        EXPECT_EQ("escape('') 返回空字符串", escaped, std::string(""));
    }

    // ── 14. escape() — 普通字符串不变 ────────────────────────
    {
        std::string raw     = "hello_world_123";
        std::string escaped = DatabaseManager::getInstance().escape(raw);
        EXPECT_EQ("escape(普通字符串) 返回原字符串", escaped, raw);
    }

    // ── 15. execute() — 无效 SQL 应抛出 ─────────────────────
    EXPECT_THROW("execute(无效SQL) 应抛出 runtime_error",
                 []() { DatabaseManager::getInstance().execute("INVALID SQL STATEMENT"); });
}

// ============================================================
// 第三部分：端到端 HTTP 测试（启动服务器线程）
// ============================================================

void testEndToEndHealthCheck() {
    std::cout << "\n[端到端] HTTP /api/v1/health-check 测试\n";

    // 服务器端口与主服务器错开，避免冲突
    const int TEST_PORT = 18081;

    std::atomic<bool> server_ready{false};
    std::atomic<bool> server_stop{false};

    // ── 在后台线程启动一个独立的测试服务器 ────────────────────
    std::thread server_thread([TEST_PORT, &server_ready, &server_stop]() {
        httplib::Server svr;

        // 注册 OPTIONS 预检
        svr.Options(".*", [](const httplib::Request&, httplib::Response& res) {
            ResponseHelper::setCorsHeaders(res);
            res.status = 204;
        });

        // 注册 health-check 路由（与 main.cpp 保持一致）
        svr.Get("/api/v1/health-check",
                [](const httplib::Request&, httplib::Response& res) {
                    ResponseHelper::setCorsHeaders(res);
                    ResponseHelper::ok(res,
                        json{{"status", "ok"}, {"version", "1.0.0"}},
                        "服务运行正常");
                });

        // 注册停止路由（测试结束后调用）
        svr.Get("/stop", [&svr](const httplib::Request&, httplib::Response& res) {
            res.status = 200;
            svr.stop();
        });

        server_ready.store(true);
        svr.listen("127.0.0.1", TEST_PORT);
    });

    // ── 等待服务器启动 ────────────────────────────────────────
    int wait_ms = 0;
    while (!server_ready.load() && wait_ms < 3000) {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        wait_ms += 50;
    }
    // 额外等待监听端口绑定完成
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    // ── 发送 GET /api/v1/health-check ─────────────────────────
    {
        httplib::Client cli("127.0.0.1", TEST_PORT);
        cli.set_connection_timeout(2);
        cli.set_read_timeout(2);
        auto result = cli.Get("/api/v1/health-check");

        EXPECT_TRUE("health-check 请求成功（非 nullptr）", result != nullptr);

        if (result) {
            EXPECT_EQ("health-check HTTP 状态码 = 200",
                      result->status, 200);

            EXPECT_CONTAINS("health-check Content-Type 含 application/json",
                            result->get_header_value("Content-Type"),
                            "application/json");

            EXPECT_CONTAINS("health-check CORS Allow-Origin = *",
                            result->get_header_value("Access-Control-Allow-Origin"),
                            "*");

            bool json_ok = false;
            try {
                json body = json::parse(result->body);
                json_ok = (body["code"].get<int>() == 200)
                       && (body["data"]["status"].get<std::string>() == "ok")
                       && (body["data"]["version"].get<std::string>() == "1.0.0")
                       && (body["msg"].get<std::string>() == "服务运行正常");
            } catch (...) {}
            EXPECT_TRUE("health-check JSON 响应结构与字段值正确", json_ok);
        }
    }

    // ── 发送 OPTIONS 预检请求 ─────────────────────────────────
    {
        httplib::Client cli("127.0.0.1", TEST_PORT);
        cli.set_connection_timeout(2);
        cli.set_read_timeout(2);
        // httplib Client 的 Options() 方法
        auto result = cli.Options("/api/v1/health-check");
        EXPECT_TRUE("OPTIONS 预检请求成功", result != nullptr);
        if (result) {
            EXPECT_EQ("OPTIONS 预检 HTTP 状态码 = 204",
                      result->status, 204);
        }
    }

    // ── 关闭测试服务器 ────────────────────────────────────────
    {
        httplib::Client cli("127.0.0.1", TEST_PORT);
        cli.set_connection_timeout(1);
        cli.Get("/stop");
    }

    if (server_thread.joinable()) {
        server_thread.join();
    }
}

// ============================================================
// main()
// ============================================================
int main() {
    std::cout << "================================================\n";
    std::cout << " 社区健康档案管理系统 — 后端基础架构测试套件\n";
    std::cout << "================================================\n";

    // ── 第一部分：ResponseHelper 单元测试 ──────────────────────
    std::cout << "\n===== 第一部分：ResponseHelper 单元测试 =====\n";
    testResponseHelperSuccess();
    testResponseHelperError();
    testResponseHelperSendJson();
    testResponseHelperCors();

    // ── 第二部分：DatabaseManager 集成测试 ─────────────────────
    std::cout << "\n===== 第二部分：DatabaseManager 集成测试 =====\n";
    testDatabaseManagerIntegration();

    // ── 第三部分：端到端 HTTP 测试 ─────────────────────────────
    std::cout << "\n===== 第三部分：端到端 HTTP 测试 =====\n";
    testEndToEndHealthCheck();

    // ── 汇总 ──────────────────────────────────────────────────
    bool all_passed = TestRunner::summary();
    return all_passed ? 0 : 1;
}
