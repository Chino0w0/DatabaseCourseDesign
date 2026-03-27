#include "DatabaseManager.h"

#include <iostream>
#include <stdexcept>
#include <vector>

// ============================================================
// 构造 / 析构
// ============================================================

DatabaseManager::DatabaseManager()
    : conn_(nullptr), initialized_(false), port_(3306) {}

DatabaseManager::~DatabaseManager() {
    // 程序退出时关闭连接
    if (conn_) {
        mysql_close(conn_);
        conn_ = nullptr;
    }
}

// ============================================================
// 单例获取
// ============================================================

DatabaseManager& DatabaseManager::getInstance() {
    // C++11 保证局部静态变量的线程安全初始化
    static DatabaseManager instance;
    return instance;
}

// ============================================================
// 初始化数据库连接
// ============================================================

void DatabaseManager::init(const std::string& host, int port,
                           const std::string& user, const std::string& password,
                           const std::string& db_name) {
    std::lock_guard<std::mutex> lock(mutex_);

    // 已初始化则直接返回，避免重复连接
    if (initialized_) {
        return;
    }

    // 保存连接参数，供断线后重连使用
    host_     = host;
    port_     = port;
    user_     = user;
    password_ = password;
    db_name_  = db_name;

    // 初始化 MySQL 客户端库
    conn_ = mysql_init(nullptr);
    if (!conn_) {
        throw std::runtime_error("MySQL 客户端库初始化失败（mysql_init 返回 NULL）");
    }

    // 设置客户端字符集为 utf8mb4，支持中文及 emoji
    mysql_options(conn_, MYSQL_SET_CHARSET_NAME, "utf8mb4");

    // 建立连接
    if (!mysql_real_connect(conn_,
                            host_.c_str(),
                            user_.c_str(),
                            password_.c_str(),
                            db_name_.c_str(),
                            static_cast<unsigned int>(port_),
                            nullptr, 0)) {
        std::string err = "MySQL 连接失败: ";
        err += mysql_error(conn_);
        mysql_close(conn_);
        conn_ = nullptr;
        throw std::runtime_error(err);
    }

    // 将会话字符集设置为 utf8mb4，确保中文正确传输
    mysql_set_character_set(conn_, "utf8mb4");

    initialized_ = true;
    std::cout << "[DB] 连接成功: " << db_name_ << "@" << host_
              << ":" << port_ << std::endl;
}

// ============================================================
// 获取连接句柄（内部含心跳检测）
// ============================================================

MYSQL* DatabaseManager::getConnection() {
    if (!initialized_ || !conn_) {
        throw std::runtime_error("数据库未初始化，请先在 main() 中调用 DatabaseManager::init()");
    }

    // mysql_ping() 返回非 0 表示连接已断开，尝试重新连接
    if (mysql_ping(conn_) != 0) {
        std::cerr << "[DB] 连接断开，尝试重新连接..." << std::endl;

        mysql_close(conn_);
        conn_ = mysql_init(nullptr);
        if (!conn_) {
            throw std::runtime_error("重连时 mysql_init 失败");
        }

        mysql_options(conn_, MYSQL_SET_CHARSET_NAME, "utf8mb4");

        if (!mysql_real_connect(conn_,
                                host_.c_str(),
                                user_.c_str(),
                                password_.c_str(),
                                db_name_.c_str(),
                                static_cast<unsigned int>(port_),
                                nullptr, 0)) {
            std::string err = "数据库重连失败: ";
            err += mysql_error(conn_);
            mysql_close(conn_);
            conn_ = nullptr;
            throw std::runtime_error(err);
        }

        mysql_set_character_set(conn_, "utf8mb4");
        std::cout << "[DB] 重连成功" << std::endl;
    }

    return conn_;
}

// ============================================================
// 查询（SELECT）— 调用方需释放结果集
// ============================================================

MYSQL_RES* DatabaseManager::query(const std::string& sql) {
    MYSQL* conn = getConnection();

    if (mysql_query(conn, sql.c_str()) != 0) {
        std::string err = "SQL 查询失败: ";
        err += mysql_error(conn);
        err += "\n SQL: " + sql;
        throw std::runtime_error(err);
    }

    MYSQL_RES* result = mysql_store_result(conn);
    if (!result && mysql_field_count(conn) > 0) {
        // 有字段但没有结果集，说明 store_result 失败
        std::string err = "获取查询结果失败: ";
        err += mysql_error(conn);
        throw std::runtime_error(err);
    }

    return result;  // 调用方负责 mysql_free_result(result)
}

// ============================================================
// 执行（INSERT / UPDATE / DELETE）
// ============================================================

void DatabaseManager::execute(const std::string& sql) {
    MYSQL* conn = getConnection();

    if (mysql_query(conn, sql.c_str()) != 0) {
        std::string err = "SQL 执行失败: ";
        err += mysql_error(conn);
        err += "\n SQL: " + sql;
        throw std::runtime_error(err);
    }
}

// ============================================================
// 获取最近一次 INSERT 的自增 ID
//
// 注意：不能通过 getConnection() 获取连接，因为 getConnection()
// 内部会调用 mysql_ping()，而 mysql_ping() 在 MySQL 8.0 中会
// 重置 mysql_insert_id() 的返回值为 0。
// 因此直接访问已保存的 conn_ 指针。
// ============================================================

unsigned long long DatabaseManager::lastInsertId() {
    if (!initialized_ || !conn_) {
        throw std::runtime_error("数据库未初始化，请先在 main() 中调用 DatabaseManager::init()");
    }
    // 直接调用 mysql_insert_id，不触发 mysql_ping()
    return mysql_insert_id(conn_);
}

// ============================================================
// SQL 字符串转义，防止注入
// ============================================================

std::string DatabaseManager::escape(const std::string& raw) {
    MYSQL* conn = getConnection();
    // mysql_real_escape_string 最多需要 2*len+1 字节的缓冲区
    std::vector<char> buf(raw.size() * 2 + 1);
    unsigned long len = mysql_real_escape_string(
        conn, buf.data(), raw.c_str(),
        static_cast<unsigned long>(raw.size()));
    return std::string(buf.data(), len);
}
