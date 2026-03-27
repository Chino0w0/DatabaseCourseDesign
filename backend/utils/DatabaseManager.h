#pragma once

#include <mutex>
#include <mysql/mysql.h>
#include <stdexcept>
#include <string>

/**
 * @brief MySQL 数据库连接管理器（单例模式）
 *
 * 整个后端共享同一个 MYSQL* 连接。所有 DAO 层代码通过
 * DatabaseManager::getInstance().getConnection() 获取连接句柄，
 * 无需自行管理连接生命周期。
 *
 * 使用方法：
 *   1. 在 main() 中调用 DatabaseManager::getInstance().init(...)
 *   2. 在 DAO 层调用 DatabaseManager::getInstance().getConnection()
 */
class DatabaseManager {
public:
  /**
   * @brief 获取单例实例
   * @return DatabaseManager 的唯一实例引用
   */
  static DatabaseManager &getInstance();

  /**
   * @brief 初始化并建立 MySQL 连接
   *
   * 只需在程序启动时调用一次。重复调用将直接返回，不会重新连接。
   *
   * @param host      数据库主机名或 IP
   * @param port      数据库端口（默认 3306）
   * @param user      数据库用户名
   * @param password  数据库密码
   * @param db_name   数据库名称
   * @throws std::runtime_error 连接失败时抛出
   */
  void init(const std::string &host, int port, const std::string &user,
            const std::string &password, const std::string &db_name);

  /**
   * @brief 获取 MySQL 连接句柄
   *
   * 在返回前会调用 mysql_ping() 检测连接存活性，自动重连。
   *
   * @return 有效的 MYSQL* 指针
   * @throws std::runtime_error 连接未初始化或重连失败时抛出
   */
  MYSQL *getConnection();

  /**
   * @brief 执行 SQL 查询（SELECT 类）并返回结果集
   *
   * 调用方负责通过 mysql_free_result() 释放结果集。
   *
   * @param sql  待执行的 SQL 字符串
   * @return MYSQL_RES* 结果集指针
   * @throws std::runtime_error SQL 执行失败时抛出
   */
  MYSQL_RES *query(const std::string &sql);

  /**
   * @brief 执行 SQL 语句（INSERT / UPDATE / DELETE 类）
   *
   * @param sql  待执行的 SQL 字符串
   * @throws std::runtime_error SQL 执行失败时抛出
   */
  void execute(const std::string &sql);

  /**
   * @brief 返回最近一次 INSERT 操作的自增 ID
   *
   * @note 本函数直接访问内部连接指针，不调用 getConnection()，
   *       以避免 getConnection() 内的 mysql_ping() 在 MySQL 8.0
   *       中会将 mysql_insert_id 重置为 0 的问题。
   *       必须在 init() 成功后使用，并紧跟在 execute(INSERT) 之后调用。
   * @return unsigned long long 自增主键值
   */
  unsigned long long lastInsertId();

  /**
   * @brief 对字符串中的特殊字符进行转义，防止 SQL 注入
   * @param raw  原始字符串
   * @return 转义后的安全字符串
   */
  std::string escape(const std::string &raw);

  // 禁止拷贝与赋值（单例必要措施）
  DatabaseManager(const DatabaseManager &) = delete;
  DatabaseManager &operator=(const DatabaseManager &) = delete;

private:
  DatabaseManager();  ///< 私有构造
  ~DatabaseManager(); ///< 私有析构，关闭连接

  /**
   * @brief 在已持有 mutex_ 的前提下获取连接句柄
   *
   * 同一个 MYSQL* 连接不能被多个线程并发调用。
   * 因此 query/execute/escape/lastInsertId 在进入底层 MySQL C API 前，
   * 都必须先持有 mutex_，再调用本函数完成存活检测与自动重连。
   */
  MYSQL *getConnectionUnlocked();

  MYSQL *conn_;      ///< MySQL 连接句柄
  bool initialized_; ///< 是否已完成初始化
  std::mutex mutex_; ///< 保护连接初始化 / 重连 / 查询执行的互斥锁

  // 断线重连时所需的配置
  std::string host_;
  int port_;
  std::string user_;
  std::string password_;
  std::string db_name_;
};
