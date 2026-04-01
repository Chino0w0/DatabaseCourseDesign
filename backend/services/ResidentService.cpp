#include "ResidentService.h"

#include "../dao/ResidentDAO.h"
#include "../models/Community.h"
#include "../models/Resident.h"
#include "../utils/DatabaseManager.h"

#include <optional>
#include <regex>
#include <sstream>
#include <stdexcept>

namespace {

bool isValidCommunityContactPhone(const std::string& phone) {
    if (phone.empty()) return true;
    static const std::regex kPhonePattern(R"(^(1[3-9]\d{9}|0\d{2,3}-?\d{7,8})$)");
    return std::regex_match(phone, kPhonePattern);
}

std::string buildResidentUsername(int resident_id) {
    return "resident_" + std::to_string(resident_id);
}

std::string buildDefaultResidentPassword(const std::string& id_card) {
    if (id_card.size() >= 6) {
        return id_card.substr(id_card.size() - 6);
    }
    return "123456";
}

std::optional<int> findResidentRoleId(DatabaseManager& db) {
    MYSQL_RES* res = db.query("SELECT id FROM roles WHERE role_name = '居民' LIMIT 1");
    if (!res) return std::nullopt;

    std::optional<int> role_id;
    MYSQL_ROW row = mysql_fetch_row(res);
    if (row && row[0]) {
        role_id = std::stoi(row[0]);
    }
    mysql_free_result(res);
    return role_id;
}

} // namespace

// ============================================================
// 内部辅助：将实体对象转换为 JSON
// ============================================================

json ResidentService::communityToJson(const Community& c) {
    return json{
        {"id",            c.id},
        {"name",          c.name},
        {"address",       c.address},
        {"contact_phone", c.contact_phone},
        {"created_at",    c.created_at}
    };
}

json ResidentService::residentToJson(const Resident& r) {
    return json{
        {"id",                r.id},
        {"name",              r.name},
        {"gender",            r.gender},
        {"id_card",           r.id_card},
        {"birth_date",        r.birth_date},
        {"phone",             r.phone},
        {"community_id",      r.community_id},
        {"community_name",    r.community_name},
        {"address",           r.address},
        {"emergency_contact", r.emergency_contact},
        {"emergency_phone",   r.emergency_phone},
        {"created_at",        r.created_at},
        {"updated_at",        r.updated_at}
    };
}

// ============================================================
// 社区业务逻辑
// ============================================================

json ResidentService::getCommunityList() {
    ResidentDAO dao;
    std::vector<Community> list = dao.listCommunities();

    json arr = json::array();
    for (const auto& c : list) {
        arr.push_back(communityToJson(c));
    }
    return arr;
}

json ResidentService::createCommunity(const json& body) {
    // 参数校验
    if (!body.contains("name") || !body["name"].is_string()
        || body["name"].get<std::string>().empty()) {
        return json{{"error", "社区名称不能为空"}};
    }

    Community c;
    c.name          = body["name"].get<std::string>();
    c.address       = body.value("address",       std::string());
    c.contact_phone = body.value("contact_phone", std::string());

    if (!isValidCommunityContactPhone(c.contact_phone)) {
        return json{{"error", "联系电话格式不正确，应为11位手机号或区号-座机号"}};
    }

    ResidentDAO dao;
    int new_id = dao.insertCommunity(c);
    return json{{"id", new_id}};
}

json ResidentService::updateCommunity(int id, const json& body) {
    ResidentDAO dao;
    Community existing = dao.getCommunityById(id);
    if (existing.id == 0) {
        return json{{"error", "社区不存在"}, {"not_found", true}};
    }

    if (body.contains("name") && body["name"].is_string()) {
        existing.name = body["name"].get<std::string>();
        if (existing.name.empty()) {
            return json{{"error", "社区名称不能为空"}};
        }
    }
    if (body.contains("address") && body["address"].is_string()) {
        existing.address = body["address"].get<std::string>();
    }
    if (body.contains("contact_phone") && body["contact_phone"].is_string()) {
        existing.contact_phone = body["contact_phone"].get<std::string>();
    }

    if (!isValidCommunityContactPhone(existing.contact_phone)) {
        return json{{"error", "联系电话格式不正确，应为11位手机号或区号-座机号"}};
    }

    dao.updateCommunity(id, existing);
    return json{{"updated", true}};
}

// ============================================================
// 居民业务逻辑
// ============================================================

json ResidentService::getResidentList(int page, int size,
                                       const std::string& keyword,
                                       int community_id) {
    if (page < 1)  page = 1;
    if (size < 1)  size = 10;
    if (size > 100) size = 100;   // 防止一次性查询过多

    ResidentDAO dao;
    int total = dao.countResidents(keyword, community_id);
    std::vector<Resident> list = dao.listResidents(page, size, keyword, community_id);

    json arr = json::array();
    for (const auto& r : list) {
        arr.push_back(residentToJson(r));
    }

    return json{
        {"total", total},
        {"page",  page},
        {"size",  size},
        {"list",  arr}
    };
}

json ResidentService::getResidentDetail(int id) {
    ResidentDAO dao;
    Resident r = dao.getResidentById(id);

    if (r.id == 0) {
        return nullptr;
    }

    return residentToJson(r);
}

json ResidentService::createResident(const json& body) {
    // 必填字段校验
    if (!body.contains("name") || !body["name"].is_string()
        || body["name"].get<std::string>().empty()) {
        return json{{"error", "姓名不能为空"}};
    }
    if (!body.contains("gender") || !body["gender"].is_string()
        || body["gender"].get<std::string>().empty()) {
        return json{{"error", "性别不能为空"}};
    }
    if (!body.contains("id_card") || !body["id_card"].is_string()
        || body["id_card"].get<std::string>().empty()) {
        return json{{"error", "身份证号不能为空"}};
    }
    if (!body.contains("birth_date") || !body["birth_date"].is_string()
        || body["birth_date"].get<std::string>().empty()) {
        return json{{"error", "出生日期不能为空"}};
    }

    std::string gender = body["gender"].get<std::string>();
    if (gender != "男" && gender != "女") {
        return json{{"error", "性别只能为'男'或'女'"}};
    }

    std::string id_card = body["id_card"].get<std::string>();

    ResidentDAO dao;
    // 身份证号唯一性检查
    if (dao.idCardExists(id_card)) {
        return json{{"error", "身份证号已存在"}};
    }

    Resident r;
    r.name              = body["name"].get<std::string>();
    r.gender            = gender;
    r.id_card           = id_card;
    r.birth_date        = body["birth_date"].get<std::string>();
    r.phone             = body.value("phone",             std::string());
    r.community_id      = body.value("community_id",      0);
    r.address           = body.value("address",           std::string());
    r.emergency_contact = body.value("emergency_contact", std::string());
    r.emergency_phone   = body.value("emergency_phone",   std::string());

    DatabaseManager& db = DatabaseManager::getInstance();

    std::string resident_username;
    std::string resident_password;
    int new_id = 0;

    try {
        db.execute("START TRANSACTION");

        new_id = dao.insertResident(r);
        resident_username = buildResidentUsername(new_id);
        resident_password = buildDefaultResidentPassword(id_card);

        auto resident_role_id = findResidentRoleId(db);
        if (!resident_role_id.has_value()) {
            throw std::runtime_error("缺少'居民'角色，请先初始化角色数据");
        }

        std::ostringstream user_sql;
        user_sql << "INSERT INTO users (username, password_hash, real_name, phone, status) VALUES ("
                 << "'" << db.escape(resident_username) << "', "
                 << "SHA2('" << db.escape(resident_password) << "', 256), "
                 << "'" << db.escape(r.name) << "', ";
        if (r.phone.empty()) {
            user_sql << "NULL";
        } else {
            user_sql << "'" << db.escape(r.phone) << "'";
        }
        user_sql << ", 1)";
        db.execute(user_sql.str());

        int new_user_id = static_cast<int>(db.lastInsertId());

        db.execute("INSERT INTO user_roles (user_id, role_id) VALUES (" +
                   std::to_string(new_user_id) + ", " +
                   std::to_string(*resident_role_id) + ")");

        db.execute("INSERT INTO resident_accounts (user_id, resident_id) VALUES (" +
                   std::to_string(new_user_id) + ", " +
                   std::to_string(new_id) + ")");

        db.execute("COMMIT");
    } catch (...) {
        try {
            db.execute("ROLLBACK");
        } catch (...) {
        }
        throw;
    }

    return json{{"id", new_id},
                {"account",
                 {{"username", resident_username},
                  {"default_password", resident_password}}}};
}

json ResidentService::updateResident(int id, const json& body) {
    ResidentDAO dao;

    if (!dao.residentExists(id)) {
        return json{{"error", "居民不存在"}, {"not_found", true}};
    }

    // 字段校验（仅在请求中存在该字段时校验）
    if (body.contains("name") && body["name"].is_string()
        && body["name"].get<std::string>().empty()) {
        return json{{"error", "姓名不能为空"}};
    }
    if (body.contains("gender") && body["gender"].is_string()) {
        std::string g = body["gender"].get<std::string>();
        if (g != "男" && g != "女") {
            return json{{"error", "性别只能为'男'或'女'"}};
        }
    }
    if (body.contains("id_card") && body["id_card"].is_string()) {
        std::string card = body["id_card"].get<std::string>();
        if (dao.idCardExists(card, id)) {
            return json{{"error", "身份证号已被其他居民使用"}};
        }
    }

    // 先获取原始数据，用请求中的字段覆盖
    Resident existing = dao.getResidentById(id);

    if (body.contains("name")              && body["name"].is_string())
        existing.name              = body["name"].get<std::string>();
    if (body.contains("gender")            && body["gender"].is_string())
        existing.gender            = body["gender"].get<std::string>();
    if (body.contains("id_card")           && body["id_card"].is_string())
        existing.id_card           = body["id_card"].get<std::string>();
    if (body.contains("birth_date")        && body["birth_date"].is_string())
        existing.birth_date        = body["birth_date"].get<std::string>();
    if (body.contains("phone")             && body["phone"].is_string())
        existing.phone             = body["phone"].get<std::string>();
    if (body.contains("community_id")      && body["community_id"].is_number_integer())
        existing.community_id      = body["community_id"].get<int>();
    if (body.contains("address")           && body["address"].is_string())
        existing.address           = body["address"].get<std::string>();
    if (body.contains("emergency_contact") && body["emergency_contact"].is_string())
        existing.emergency_contact = body["emergency_contact"].get<std::string>();
    if (body.contains("emergency_phone")   && body["emergency_phone"].is_string())
        existing.emergency_phone   = body["emergency_phone"].get<std::string>();

    dao.updateResident(id, existing);
    return json{{"updated", true}};
}

bool ResidentService::deleteResident(int id) {
    ResidentDAO dao;
    return dao.deleteResident(id);
}
