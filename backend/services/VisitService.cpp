#include "VisitService.h"

#include "../dao/VisitLogDAO.h"

namespace {

json visitToJson(const VisitLog& v) {
    return json{
        {"id", v.id},
        {"resident_id", v.resident_id},
        {"visitor_user_id", v.visitor_user_id},
        {"visitor_name", v.visitor_name.empty() ? json(nullptr) : json(v.visitor_name)},
        {"visit_type", v.visit_type.empty() ? json(nullptr) : json(v.visit_type)},
        {"visit_date", v.visit_date},
        {"content", v.content.empty() ? json(nullptr) : json(v.content)},
        {"next_visit_date", v.next_visit_date.empty() ? json(nullptr) : json(v.next_visit_date)},
        {"created_at", v.created_at.empty() ? json(nullptr) : json(v.created_at)}
    };
}

} // namespace

json VisitService::getVisits(int resident_id) {
    if (resident_id <= 0) {
        return json{{"error", "resident_id 必须为正整数"}};
    }

    VisitLogDAO dao;
    if (!dao.residentExists(resident_id)) {
        return json{{"error", "居民不存在"}, {"not_found", true}};
    }

    auto list = dao.listByResidentId(resident_id);
    json arr = json::array();
    for (const auto& item : list) {
        arr.push_back(visitToJson(item));
    }
    return arr;
}

json VisitService::createVisit(const json& body) {
    if (!body.contains("resident_id") || !body["resident_id"].is_number_integer()) {
        return json{{"error", "resident_id 必须为整数"}};
    }
    if (!body.contains("visitor_user_id") || !body["visitor_user_id"].is_number_integer()) {
        return json{{"error", "visitor_user_id 必须为整数"}};
    }
    if (!body.contains("visit_type") || !body["visit_type"].is_string()
        || body["visit_type"].get<std::string>().empty()) {
        return json{{"error", "visit_type 不能为空"}};
    }
    if (!body.contains("visit_date") || !body["visit_date"].is_string()
        || body["visit_date"].get<std::string>().empty()) {
        return json{{"error", "visit_date 不能为空"}};
    }

    const int resident_id = body["resident_id"].get<int>();
    const int visitor_user_id = body["visitor_user_id"].get<int>();
    if (resident_id <= 0) return json{{"error", "resident_id 必须为正整数"}};
    if (visitor_user_id <= 0) return json{{"error", "visitor_user_id 必须为正整数"}};

    std::string content;
    if (body.contains("content") && !body["content"].is_null()) {
        if (!body["content"].is_string()) {
            return json{{"error", "content 必须为字符串"}};
        }
        content = body["content"].get<std::string>();
    }

    std::string next_visit_date;
    if (body.contains("next_visit_date") && !body["next_visit_date"].is_null()) {
        if (!body["next_visit_date"].is_string()) {
            return json{{"error", "next_visit_date 必须为字符串"}};
        }
        next_visit_date = body["next_visit_date"].get<std::string>();
    }

    VisitLogDAO dao;
    if (!dao.residentExists(resident_id)) {
        return json{{"error", "居民不存在"}, {"not_found", true}};
    }
    if (!dao.userExists(visitor_user_id)) {
        return json{{"error", "随访人员不存在"}, {"not_found", true}};
    }

    VisitLog log;
    log.resident_id = resident_id;
    log.visitor_user_id = visitor_user_id;
    log.visit_type = body["visit_type"].get<std::string>();
    log.visit_date = body["visit_date"].get<std::string>();
    log.content = content;
    log.next_visit_date = next_visit_date;

    const int new_id = dao.insertVisitLog(log);
    return json{{"id", new_id}};
}
