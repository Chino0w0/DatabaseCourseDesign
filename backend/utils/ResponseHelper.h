#pragma once

#include <string>

// 固定使用项目本地副本，减少 IDE 误报
#include "../include/httplib.h"
#include "../include/json.hpp"


using json = nlohmann::json;

/**
 * @brief 统一 JSON 响应格式 & CORS 中间件工具集
 *
 * 所有控制器通过本命名空间构造并发送响应，保证全局格式一致：
 * {
 *   "code": 200,        // 状态码
 *   "msg":  "操作成功",  // 提示信息
 *   "data": { ... }     // 业务数据（可为 null）
 * }
 */
namespace ResponseHelper {

// --------------------------------------------------------
// 构造标准响应体
// --------------------------------------------------------

/**
 * @brief 构造成功响应体（code = 200）
 * @param data  业务数据，默认为 null
 * @param msg   提示信息，默认"操作成功"
 * @return json 对象
 */
inline json success(const json &data = nullptr,
                    const std::string &msg = "操作成功") {
  return json{{"code", 200}, {"msg", msg}, {"data", data}};
}

/**
 * @brief 构造错误响应体
 * @param code  错误码（400 参数错误 / 401 未认证 / 404 不存在 / 500
 * 服务器错误）
 * @param msg   错误描述信息
 * @return json 对象
 */
inline json error(int code, const std::string &msg) {
  return json{{"code", code}, {"msg", msg}, {"data", nullptr}};
}

// --------------------------------------------------------
// 发送响应
// --------------------------------------------------------

/**
 * @brief 将 json 对象序列化后写入 HTTP 响应
 * @param res     cpp-httplib 响应对象（引用）
 * @param body    待发送的 json 对象
 * @param status  HTTP 状态码，默认 200
 */
inline void sendJson(httplib::Response &res, const json &body,
                     int status = 200) {
  res.status = status;
  res.set_content(body.dump(), "application/json; charset=utf-8");
}

/**
 * @brief 发送成功响应（一步到位的便捷函数）
 * @param res   HTTP 响应对象
 * @param data  业务数据
 * @param msg   提示信息
 */
inline void ok(httplib::Response &res, const json &data = nullptr,
               const std::string &msg = "操作成功") {
  sendJson(res, success(data, msg), 200);
}

/**
 * @brief 发送错误响应（一步到位的便捷函数）
 * @param res   HTTP 响应对象
 * @param code  业务错误码
 * @param msg   错误描述
 */
inline void fail(httplib::Response &res, int code, const std::string &msg) {
  sendJson(res, error(code, msg), code >= 500 ? 500 : code);
}

// --------------------------------------------------------
// CORS 跨域支持
// --------------------------------------------------------

/**
 * @brief 为 HTTP 响应设置 CORS 跨域头
 *
 * 允许前端页面（浏览器）跨域访问后端 API。
 * 开发阶段 origin 设置为 *，生产环境可改为具体域名。
 *
 * @param res  HTTP 响应对象
 */
inline void setCorsHeaders(httplib::Response &res) {
  res.set_header("Access-Control-Allow-Origin", "*");
  res.set_header("Access-Control-Allow-Methods",
                 "GET, POST, PUT, DELETE, OPTIONS");
  res.set_header("Access-Control-Allow-Headers", "Content-Type, Authorization");
  res.set_header("Access-Control-Max-Age", "86400");
}

} // namespace ResponseHelper
