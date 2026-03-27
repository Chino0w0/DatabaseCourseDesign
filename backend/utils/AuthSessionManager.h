#pragma once

#include "../dao/UserDAO.h"
#include "ResponseHelper.h"

#include <algorithm>
#include <iomanip>
#include <mutex>
#include <optional>
#include <random>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

namespace AuthSessionManager {

struct SessionInfo {
  std::string token;
  int userId = 0;
};

inline std::mutex g_sessionMutex;
inline std::unordered_map<std::string, SessionInfo> g_sessions;

inline std::string generateToken() {
  static std::random_device rd;
  static std::mt19937_64 gen(rd());
  static std::uniform_int_distribution<unsigned long long> dist;

  std::ostringstream oss;
  oss << std::hex << std::setfill('0');
  for (int i = 0; i < 4; ++i) {
    oss << std::setw(16) << dist(gen);
  }
  return oss.str();
}

inline std::string issueToken(const User &user) {
  std::lock_guard<std::mutex> lock(g_sessionMutex);
  std::string token;
  do {
    token = generateToken();
  } while (g_sessions.find(token) != g_sessions.end());

  g_sessions[token] = SessionInfo{token, user.id};
  return token;
}

inline void revokeToken(const std::string &token) {
  std::lock_guard<std::mutex> lock(g_sessionMutex);
  g_sessions.erase(token);
}

inline void revokeUserTokens(int userId) {
  std::lock_guard<std::mutex> lock(g_sessionMutex);
  for (auto it = g_sessions.begin(); it != g_sessions.end();) {
    if (it->second.userId == userId) {
      it = g_sessions.erase(it);
    } else {
      ++it;
    }
  }
}

inline std::string extractBearerToken(const httplib::Request &req) {
  if (!req.has_header("Authorization")) {
    return "";
  }

  const std::string header = req.get_header_value("Authorization");
  const std::string prefix = "Bearer ";
  if (header.size() <= prefix.size() || header.rfind(prefix, 0) != 0) {
    return "";
  }

  return header.substr(prefix.size());
}

inline std::optional<User> authenticateToken(const std::string &token) {
  if (token.empty()) {
    return std::nullopt;
  }

  int userId = 0;
  {
    std::lock_guard<std::mutex> lock(g_sessionMutex);
    auto it = g_sessions.find(token);
    if (it == g_sessions.end()) {
      return std::nullopt;
    }
    userId = it->second.userId;
  }

  UserDAO dao;
  auto userOpt = dao.findById(userId);
  if (!userOpt.has_value() || userOpt->status != 1) {
    revokeToken(token);
    return std::nullopt;
  }

  return userOpt;
}

inline bool hasRequiredRole(const User &user,
                            const std::vector<int> &allowedRoleIds) {
  return allowedRoleIds.empty() ||
         std::find(allowedRoleIds.begin(), allowedRoleIds.end(), user.roleId) !=
             allowedRoleIds.end();
}

inline std::optional<User>
requireUser(const httplib::Request &req, httplib::Response &res,
            const std::vector<int> &allowedRoleIds = {}) {
  const std::string token = extractBearerToken(req);
  if (token.empty()) {
    ResponseHelper::setCorsHeaders(res);
    ResponseHelper::fail(res, 401, "未登录或登录已过期");
    return std::nullopt;
  }

  auto userOpt = authenticateToken(token);
  if (!userOpt.has_value()) {
    ResponseHelper::setCorsHeaders(res);
    ResponseHelper::fail(res, 401, "当前用户已被禁用或登录已失效");
    return std::nullopt;
  }

  if (!hasRequiredRole(*userOpt, allowedRoleIds)) {
    ResponseHelper::setCorsHeaders(res);
    ResponseHelper::fail(res, 403, "无权限访问该接口");
    return std::nullopt;
  }

  return userOpt;
}

} // namespace AuthSessionManager
