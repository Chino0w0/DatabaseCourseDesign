#include "HealthService.h"

#include "../dao/HealthMeasurementDAO.h"

namespace {

json nullableInt(int v) {
    return (v >= 0) ? json(v) : json(nullptr);
}

json nullableDouble(double v) {
    return (v >= 0.0) ? json(v) : json(nullptr);
}

json measurementToJson(const HealthMeasurement& m) {
    return json{
        {"id", m.id},
        {"resident_id", m.resident_id},
        {"systolic", nullableInt(m.systolic)},
        {"diastolic", nullableInt(m.diastolic)},
        {"blood_sugar", nullableDouble(m.blood_sugar)},
        {"heart_rate", nullableInt(m.heart_rate)},
        {"height", nullableDouble(m.height)},
        {"weight", nullableDouble(m.weight)},
        {"bmi", nullableDouble(m.bmi)},
        {"notes", m.notes},
        {"measured_by_user_id", (m.measured_by > 0) ? json(m.measured_by) : json(nullptr)},
        {"measured_by", m.measured_by_name.empty() ? json(nullptr) : json(m.measured_by_name)},
        {"measured_at", m.measured_at}
    };
}

json healthRecordToJson(const HealthRecord& r) {
    return json{
        {"id", r.id},
        {"resident_id", r.resident_id},
        {"blood_type", r.blood_type.empty() ? json(nullptr) : json(r.blood_type)},
        {"allergy_history", r.allergy_history.empty() ? json(nullptr) : json(r.allergy_history)},
        {"family_history", r.family_history.empty() ? json(nullptr) : json(r.family_history)},
        {"past_medical_history", r.past_medical_history.empty() ? json(nullptr) : json(r.past_medical_history)},
        {"created_at", r.created_at},
        {"updated_at", r.updated_at}
    };
}

bool optionalIntInRange(const json& body, const std::string& key, int low, int high) {
    if (!body.contains(key) || body[key].is_null()) return true;
    if (!body[key].is_number_integer()) return false;
    const int v = body[key].get<int>();
    return v >= low && v <= high;
}

bool optionalNumberInRange(const json& body, const std::string& key, double low, double high) {
    if (!body.contains(key) || body[key].is_null()) return true;
    if (!body[key].is_number()) return false;
    const double v = body[key].get<double>();
    return v >= low && v <= high;
}

} // namespace

json HealthService::getMeasurements(int resident_id, int limit) {
    if (resident_id <= 0) return json{{"error", "resident_id 必须为正整数"}};

    HealthMeasurementDAO dao;
    if (!dao.residentExists(resident_id)) {
        return json{{"error", "居民不存在"}, {"not_found", true}};
    }

    auto list = dao.listByResidentId(resident_id, limit);
    json arr = json::array();
    for (const auto& item : list) {
        json row = measurementToJson(item);
        auto warnings = dao.getWarningMessagesByMeasurementId(item.id);
        row["warning"] = !warnings.empty();
        row["warning_msg"] = warnings.empty() ? json(nullptr) : json(warnings);
        arr.push_back(row);
    }
    return arr;
}

json HealthService::createMeasurement(const json& body) {
    if (!body.contains("resident_id") || !body["resident_id"].is_number_integer()) {
        return json{{"error", "resident_id 必须为整数"}};
    }
    const int resident_id = body["resident_id"].get<int>();
    if (resident_id <= 0) return json{{"error", "resident_id 必须为正整数"}};

    if (body.contains("measured_by_user_id") && !body["measured_by_user_id"].is_null()
        && !body["measured_by_user_id"].is_number_integer()) {
        return json{{"error", "measured_by_user_id 必须为整数"}};
    }
    if (body.contains("measured_at") && !body["measured_at"].is_null()
        && !body["measured_at"].is_string()) {
        return json{{"error", "measured_at 必须为字符串（格式：YYYY-MM-DD HH:mm:ss）"}};
    }

    if (!optionalIntInRange(body, "systolic", 50, 300)) return json{{"error", "systolic 范围应为 50-300"}};
    if (!optionalIntInRange(body, "diastolic", 30, 200)) return json{{"error", "diastolic 范围应为 30-200"}};
    if (!optionalIntInRange(body, "heart_rate", 20, 260)) return json{{"error", "heart_rate 范围应为 20-260"}};
    if (!optionalNumberInRange(body, "blood_sugar", 1.0, 40.0)) return json{{"error", "blood_sugar 范围应为 1.0-40.0"}};
    if (!optionalNumberInRange(body, "height", 30.0, 260.0)) return json{{"error", "height 范围应为 30-260(cm)"}};
    if (!optionalNumberInRange(body, "weight", 1.0, 500.0)) return json{{"error", "weight 范围应为 1-500(kg)"}};

    HealthMeasurementDAO dao;
    if (!dao.residentExists(resident_id)) {
        return json{{"error", "居民不存在"}, {"not_found", true}};
    }

    int measured_by = -1;
    if (body.contains("measured_by_user_id") && !body["measured_by_user_id"].is_null()) {
        measured_by = body["measured_by_user_id"].get<int>();
        if (measured_by <= 0 || !dao.userExists(measured_by)) {
            return json{{"error", "测量人员不存在"}};
        }
    }

    HealthMeasurement m;
    m.resident_id = resident_id;
    if (body.contains("systolic") && !body["systolic"].is_null()) m.systolic = body["systolic"].get<int>();
    if (body.contains("diastolic") && !body["diastolic"].is_null()) m.diastolic = body["diastolic"].get<int>();
    if (body.contains("blood_sugar") && !body["blood_sugar"].is_null()) m.blood_sugar = body["blood_sugar"].get<double>();
    if (body.contains("heart_rate") && !body["heart_rate"].is_null()) m.heart_rate = body["heart_rate"].get<int>();
    if (body.contains("height") && !body["height"].is_null()) m.height = body["height"].get<double>();
    if (body.contains("weight") && !body["weight"].is_null()) m.weight = body["weight"].get<double>();
    m.notes = body.value("notes", std::string());
    m.measured_by = measured_by;
    if (body.contains("measured_at") && body["measured_at"].is_string()) {
        m.measured_at = body["measured_at"].get<std::string>();
    }

    const int new_id = dao.insertMeasurement(m);
    auto warnings = dao.getWarningMessagesByMeasurementId(new_id);
    return json{
        {"id", new_id},
        {"warning", !warnings.empty()},
        {"warning_msg", warnings.empty() ? json(nullptr) : json(warnings)}
    };
}

json HealthService::getHealthRecord(int resident_id) {
    if (resident_id <= 0) return json{{"error", "resident_id 必须为正整数"}};

    HealthMeasurementDAO dao;
    if (!dao.residentExists(resident_id)) {
        return json{{"error", "居民不存在"}, {"not_found", true}};
    }

    HealthRecord record = dao.getHealthRecordByResidentId(resident_id);
    if (record.id == 0) return nullptr;
    return healthRecordToJson(record);
}

json HealthService::getWarningSummary() {
    HealthMeasurementDAO dao;
    return json{
        {"total", dao.countWarnings(false)},
        {"unhandled", dao.countWarnings(true)}
    };
}

json HealthService::upsertHealthRecord(int resident_id, const json& body) {
    if (resident_id <= 0) {
        return json{{"error", "resident_id 必须为正整数"}};
    }

    auto checkOptionalString = [&body](const std::string& key) -> bool {
        return !body.contains(key) || body[key].is_null() || body[key].is_string();
    };

    if (!checkOptionalString("blood_type") ||
        !checkOptionalString("allergy_history") ||
        !checkOptionalString("family_history") ||
        !checkOptionalString("past_medical_history")) {
        return json{{"error", "摘要字段必须为字符串"}};
    }

    HealthMeasurementDAO dao;
    if (!dao.residentExists(resident_id)) {
        return json{{"error", "居民不存在"}, {"not_found", true}};
    }

    const std::string blood_type =
        (body.contains("blood_type") && !body["blood_type"].is_null())
            ? body["blood_type"].get<std::string>()
            : std::string();
    const std::string allergy_history =
        (body.contains("allergy_history") && !body["allergy_history"].is_null())
            ? body["allergy_history"].get<std::string>()
            : std::string();
    const std::string family_history =
        (body.contains("family_history") && !body["family_history"].is_null())
            ? body["family_history"].get<std::string>()
            : std::string();
    const std::string past_medical_history =
        (body.contains("past_medical_history") && !body["past_medical_history"].is_null())
            ? body["past_medical_history"].get<std::string>()
            : std::string();

    dao.upsertHealthRecord(resident_id, blood_type, allergy_history, family_history,
                           past_medical_history);

    return json{{"resident_id", resident_id}};
}
