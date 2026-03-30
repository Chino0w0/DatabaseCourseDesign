#include "HealthMeasurementDAO.h"

#include "../utils/DatabaseManager.h"

#include <sstream>

namespace {

std::string safeStr(const char* s) {
    return (s != nullptr) ? std::string(s) : std::string();
}

std::string sqlNullableInt(int v) {
    return (v >= 0) ? std::to_string(v) : "NULL";
}

std::string sqlNullableDouble(double v) {
    if (v < 0.0) return "NULL";
    std::ostringstream oss;
    oss << v;
    return oss.str();
}

std::string sqlNullableDateTime(const std::string& v) {
    if (v.empty()) return "CURRENT_TIMESTAMP";
    return "'" + DatabaseManager::getInstance().escape(v) + "'";
}

} // namespace

std::vector<HealthMeasurement> HealthMeasurementDAO::listByResidentId(int resident_id, int limit) {
    if (limit < 1) limit = 50;
    if (limit > 200) limit = 200;

    std::ostringstream sql;
    sql << "SELECT hm.id, hm.resident_id, hm.systolic, hm.diastolic, hm.blood_sugar, "
           "hm.heart_rate, hm.height, hm.weight, hm.bmi, hm.notes, hm.measured_by, "
           "hm.measured_at, IFNULL(u.real_name, '') AS measured_by_name "
           "FROM health_measurements hm "
           "LEFT JOIN users u ON hm.measured_by = u.id "
           "WHERE hm.resident_id = " << resident_id << " "
           "ORDER BY hm.measured_at DESC, hm.id DESC "
           "LIMIT " << limit;

    MYSQL_RES* res = DatabaseManager::getInstance().query(sql.str());
    std::vector<HealthMeasurement> list;
    if (!res) return list;

    MYSQL_ROW row;
    while ((row = mysql_fetch_row(res)) != nullptr) {
        HealthMeasurement m;
        m.id               = row[0]  ? std::stoi(row[0])  : 0;
        m.resident_id      = row[1]  ? std::stoi(row[1])  : 0;
        m.systolic         = row[2]  ? std::stoi(row[2])  : -1;
        m.diastolic        = row[3]  ? std::stoi(row[3])  : -1;
        m.blood_sugar      = row[4]  ? std::stod(row[4])  : -1.0;
        m.heart_rate       = row[5]  ? std::stoi(row[5])  : -1;
        m.height           = row[6]  ? std::stod(row[6])  : -1.0;
        m.weight           = row[7]  ? std::stod(row[7])  : -1.0;
        m.bmi              = row[8]  ? std::stod(row[8])  : -1.0;
        m.notes            = safeStr(row[9]);
        m.measured_by      = row[10] ? std::stoi(row[10]) : -1;
        m.measured_at      = safeStr(row[11]);
        m.measured_by_name = safeStr(row[12]);
        list.push_back(m);
    }

    mysql_free_result(res);
    return list;
}

int HealthMeasurementDAO::insertMeasurement(const HealthMeasurement& m) {
    auto& db = DatabaseManager::getInstance();

    double bmi = -1.0;
    if (m.height > 0.0 && m.weight > 0.0) {
        double meter = m.height / 100.0;
        bmi = m.weight / (meter * meter);
    }

    std::ostringstream sql;
    sql << "INSERT INTO health_measurements "
           "(resident_id, systolic, diastolic, blood_sugar, heart_rate, height, "
           "weight, bmi, notes, measured_by, measured_at) VALUES ("
        << m.resident_id << ", "
        << sqlNullableInt(m.systolic) << ", "
        << sqlNullableInt(m.diastolic) << ", "
        << sqlNullableDouble(m.blood_sugar) << ", "
        << sqlNullableInt(m.heart_rate) << ", "
        << sqlNullableDouble(m.height) << ", "
        << sqlNullableDouble(m.weight) << ", "
        << sqlNullableDouble(bmi) << ", "
        << "'" << db.escape(m.notes) << "', "
        << ((m.measured_by > 0) ? std::to_string(m.measured_by) : "NULL") << ", "
        << sqlNullableDateTime(m.measured_at)
        << ")";

    db.execute(sql.str());
    return static_cast<int>(db.lastInsertId());
}

HealthRecord HealthMeasurementDAO::getHealthRecordByResidentId(int resident_id) {
    std::string sql =
        "SELECT id, resident_id, blood_type, allergy_history, family_history, "
        "past_medical_history, created_at, updated_at "
        "FROM health_records WHERE resident_id = " + std::to_string(resident_id) + " LIMIT 1";

    MYSQL_RES* res = DatabaseManager::getInstance().query(sql);
    HealthRecord r;
    if (!res) return r;

    MYSQL_ROW row = mysql_fetch_row(res);
    if (row) {
        r.id                   = row[0] ? std::stoi(row[0]) : 0;
        r.resident_id          = row[1] ? std::stoi(row[1]) : 0;
        r.blood_type           = safeStr(row[2]);
        r.allergy_history      = safeStr(row[3]);
        r.family_history       = safeStr(row[4]);
        r.past_medical_history = safeStr(row[5]);
        r.created_at           = safeStr(row[6]);
        r.updated_at           = safeStr(row[7]);
    }

    mysql_free_result(res);
    return r;
}

bool HealthMeasurementDAO::residentExists(int resident_id) {
    std::string sql =
        "SELECT 1 FROM residents WHERE id = " + std::to_string(resident_id) + " LIMIT 1";
    MYSQL_RES* res = DatabaseManager::getInstance().query(sql);
    bool exists = false;
    if (res) {
        exists = (mysql_fetch_row(res) != nullptr);
        mysql_free_result(res);
    }
    return exists;
}

bool HealthMeasurementDAO::userExists(int user_id) {
    std::string sql =
        "SELECT 1 FROM users WHERE id = " + std::to_string(user_id) + " LIMIT 1";
    MYSQL_RES* res = DatabaseManager::getInstance().query(sql);
    bool exists = false;
    if (res) {
        exists = (mysql_fetch_row(res) != nullptr);
        mysql_free_result(res);
    }
    return exists;
}

std::vector<std::string> HealthMeasurementDAO::getWarningMessagesByMeasurementId(int measurement_id) {
    std::string sql =
        "SELECT warning_type, warning_msg FROM health_warnings WHERE measurement_id = " +
        std::to_string(measurement_id) + " ORDER BY id ASC";

    MYSQL_RES* res = nullptr;
    try {
        res = DatabaseManager::getInstance().query(sql);
    } catch (...) {
        return {};
    }

    std::vector<std::string> warnings;
    if (!res) return warnings;

    MYSQL_ROW row;
    while ((row = mysql_fetch_row(res)) != nullptr) {
        const std::string type = safeStr(row[0]);
        const std::string msg = safeStr(row[1]);
        if (!type.empty() && !msg.empty()) {
            warnings.push_back(type + ": " + msg);
        } else if (!msg.empty()) {
            warnings.push_back(msg);
        } else if (!type.empty()) {
            warnings.push_back(type);
        }
    }
    mysql_free_result(res);
    return warnings;
}

int HealthMeasurementDAO::countWarnings(bool only_unhandled) {
    std::string sql = "SELECT COUNT(*) FROM health_warnings";
    if (only_unhandled) {
        sql += " WHERE is_handled = 0";
    }

    MYSQL_RES* res = nullptr;
    try {
        res = DatabaseManager::getInstance().query(sql);
    } catch (...) {
        return 0;
    }

    if (!res) return 0;

    int count = 0;
    MYSQL_ROW row = mysql_fetch_row(res);
    if (row && row[0]) {
        count = std::stoi(row[0]);
    }

    mysql_free_result(res);
    return count;
}

bool HealthMeasurementDAO::upsertHealthRecord(
    int resident_id,
    const std::string& blood_type,
    const std::string& allergy_history,
    const std::string& family_history,
    const std::string& past_medical_history) {
    auto& db = DatabaseManager::getInstance();

    const std::string escaped_blood_type = db.escape(blood_type);
    const std::string escaped_allergy = db.escape(allergy_history);
    const std::string escaped_family = db.escape(family_history);
    const std::string escaped_past = db.escape(past_medical_history);

    std::string check_sql =
        "SELECT id FROM health_records WHERE resident_id = " +
        std::to_string(resident_id) + " LIMIT 1";

    MYSQL_RES* res = db.query(check_sql);
    const bool exists = (res && mysql_fetch_row(res) != nullptr);
    if (res) {
        mysql_free_result(res);
    }

    std::ostringstream sql;
    if (exists) {
        sql << "UPDATE health_records SET "
            << "blood_type='" << escaped_blood_type << "', "
            << "allergy_history='" << escaped_allergy << "', "
            << "family_history='" << escaped_family << "', "
            << "past_medical_history='" << escaped_past << "' "
            << "WHERE resident_id=" << resident_id;
    } else {
        sql << "INSERT INTO health_records "
            << "(resident_id, blood_type, allergy_history, family_history, past_medical_history) VALUES ("
            << resident_id << ", '"
            << escaped_blood_type << "', '"
            << escaped_allergy << "', '"
            << escaped_family << "', '"
            << escaped_past << "')";
    }

    db.execute(sql.str());
    return true;
}
