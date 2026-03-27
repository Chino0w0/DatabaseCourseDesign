#include "DiseaseService.h"

#include "../dao/DiseaseDAO.h"

namespace {

const char* const kDefaultStatus = "治疗中";
const char* const kStatusTreating = "治疗中";
const char* const kStatusRecovered = "已痊愈";
const char* const kStatusControlled = "控制中";

json diseaseToJson(const ChronicDisease& d) {
    return json{
        {"id", d.id},
        {"disease_name", d.disease_name},
        {"disease_code", d.disease_code.empty() ? json(nullptr) : json(d.disease_code)},
        {"category", d.category.empty() ? json(nullptr) : json(d.category)},
        {"description", d.description.empty() ? json(nullptr) : json(d.description)}
    };
}

json residentDiseaseToJson(const ResidentDisease& d) {
    return json{
        {"id", d.id},
        {"resident_id", d.resident_id},
        {"disease_id", d.disease_id},
        {"disease_name", d.disease_name},
        {"disease_code", d.disease_code.empty() ? json(nullptr) : json(d.disease_code)},
        {"category", d.category.empty() ? json(nullptr) : json(d.category)},
        {"diagnosed_date", d.diagnosed_date.empty() ? json(nullptr) : json(d.diagnosed_date)},
        {"status", d.status.empty() ? json(nullptr) : json(d.status)},
        {"notes", d.notes.empty() ? json(nullptr) : json(d.notes)}
    };
}

bool validStatus(const std::string& status) {
    return status == kStatusTreating || status == kStatusRecovered || status == kStatusControlled;
}

} // namespace

json DiseaseService::getDiseaseList() {
    DiseaseDAO dao;
    auto list = dao.listDiseases();

    json arr = json::array();
    for (const auto& item : list) {
        arr.push_back(diseaseToJson(item));
    }
    return arr;
}

json DiseaseService::addResidentDisease(int resident_id, const json& body) {
    if (resident_id <= 0) {
        return json{{"error", "resident_id 必须为正整数"}};
    }
    if (!body.contains("disease_id") || !body["disease_id"].is_number_integer()) {
        return json{{"error", "disease_id 必须为整数"}};
    }

    const int disease_id = body["disease_id"].get<int>();
    if (disease_id <= 0) {
        return json{{"error", "disease_id 必须为正整数"}};
    }

    std::string diagnosed_date;
    if (body.contains("diagnosed_date") && !body["diagnosed_date"].is_null()) {
        if (!body["diagnosed_date"].is_string() || body["diagnosed_date"].get<std::string>().empty()) {
            return json{{"error", "diagnosed_date 必须为非空字符串"}};
        }
        diagnosed_date = body["diagnosed_date"].get<std::string>();
    }

    std::string status = body.value("status", std::string(kDefaultStatus));
    if (!validStatus(status)) {
        return json{{"error", "status 仅支持: 治疗中/已痊愈/控制中"}};
    }

    std::string notes;
    if (body.contains("notes") && !body["notes"].is_null()) {
        if (!body["notes"].is_string()) {
            return json{{"error", "notes 必须为字符串"}};
        }
        notes = body["notes"].get<std::string>();
    }

    DiseaseDAO dao;
    if (!dao.residentExists(resident_id)) {
        return json{{"error", "居民不存在"}, {"not_found", true}};
    }
    if (!dao.diseaseExists(disease_id)) {
        return json{{"error", "慢性病类型不存在"}, {"not_found", true}};
    }
    if (dao.residentDiseaseExists(resident_id, disease_id)) {
        return json{{"error", "该居民已关联该慢性病"}};
    }

    const int new_id = dao.addResidentDisease(resident_id, disease_id, diagnosed_date, status, notes);
    return json{{"id", new_id}};
}

json DiseaseService::getResidentDiseases(int resident_id) {
    if (resident_id <= 0) {
        return json{{"error", "resident_id 必须为正整数"}};
    }

    DiseaseDAO dao;
    if (!dao.residentExists(resident_id)) {
        return json{{"error", "居民不存在"}, {"not_found", true}};
    }

    auto list = dao.listResidentDiseases(resident_id);
    json arr = json::array();
    for (const auto& item : list) {
        arr.push_back(residentDiseaseToJson(item));
    }
    return arr;
}
