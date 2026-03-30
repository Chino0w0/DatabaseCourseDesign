#include "HealthMeasurementDAO.h"

#include "../utils/DatabaseManager.h"

#include <sstream>

namespace {

std::string safeStr(const char *s) {
  return (s != nullptr) ? std::string(s) : std::string();
}

std::string sqlNullableInt(int v) {
  return (v >= 0) ? std::to_string(v) : "NULL";
}

std::string sqlNullableDouble(double v) {
  if (v < 0.0)
    return "NULL";
  std::ostringstream oss;
  oss << v;
  return oss.str();
}

std::string sqlQuotedOrNull(DatabaseManager &db, const std::string &value) {
  return value.empty() ? "NULL" : ("'" + db.escape(value) + "'");
}

void insertWarning(DatabaseManager &db, int measurement_id, int resident_id,
                   const std::string &warningType,
                   const std::string &warningMsg,
                   const std::string &warningLevel) {
  std::ostringstream sql;
  sql << "INSERT INTO health_warnings "
         "(measurement_id, resident_id, warning_type, warning_msg, "
         "warning_level) "
         "VALUES ("
      << measurement_id << ", " << resident_id << ", '"
      << db.escape(warningType) << "', '" << db.escape(warningMsg) << "', '"
      << db.escape(warningLevel) << "')";
  db.execute(sql.str());
}

void rebuildWarnings(DatabaseManager &db, int measurement_id,
                     const HealthMeasurement &m) {
  db.execute("DELETE FROM health_warnings WHERE measurement_id = " +
             std::to_string(measurement_id));

  if (m.systolic >= 140 || m.diastolic >= 90) {
    std::string level = "低";
    if (m.systolic >= 180 || m.diastolic >= 110) {
      level = "高";
    } else if (m.systolic >= 160 || m.diastolic >= 100) {
      level = "中";
    }
    insertWarning(db, measurement_id, m.resident_id, "高血压",
                  "收缩压: " + std::to_string(m.systolic) +
                      " mmHg, 舒张压: " + std::to_string(m.diastolic) + " mmHg",
                  level);
  }

  if (m.systolic >= 0 && (m.systolic <= 90 || m.diastolic <= 60)) {
    insertWarning(db, measurement_id, m.resident_id, "低血压",
                  "收缩压: " + std::to_string(m.systolic) +
                      " mmHg, 舒张压: " + std::to_string(m.diastolic) + " mmHg",
                  "中");
  }

  if (m.blood_sugar >= 7.0) {
    insertWarning(db, measurement_id, m.resident_id, "高血糖",
                  "血糖值: " + std::to_string(m.blood_sugar) + " mmol/L",
                  m.blood_sugar >= 11.1 ? "高" : "中");
  }

  if (m.blood_sugar >= 0.0 && m.blood_sugar <= 3.9) {
    insertWarning(db, measurement_id, m.resident_id, "低血糖",
                  "血糖值: " + std::to_string(m.blood_sugar) + " mmol/L", "高");
  }

  if (m.heart_rate >= 0 && (m.heart_rate > 100 || m.heart_rate < 60)) {
    insertWarning(db, measurement_id, m.resident_id, "心率异常",
                  "心率: " + std::to_string(m.heart_rate) + " 次/分",
                  (m.heart_rate > 120 || m.heart_rate < 50) ? "高" : "低");
  }
}

} // namespace

std::vector<HealthMeasurement>
HealthMeasurementDAO::listByResidentId(int resident_id, int limit) {
  if (limit < 1)
    limit = 50;
  if (limit > 200)
    limit = 200;

  std::ostringstream sql;
  sql << "SELECT hm.id, hm.resident_id, hm.systolic, hm.diastolic, "
         "hm.blood_sugar, "
         "hm.heart_rate, hm.height, hm.weight, hm.bmi, hm.notes, "
         "hm.measured_by, "
         "hm.measured_at, IFNULL(u.real_name, '') AS measured_by_name "
         "FROM health_measurements hm "
         "LEFT JOIN users u ON hm.measured_by = u.id "
         "WHERE hm.resident_id = "
      << resident_id
      << " "
         "ORDER BY hm.measured_at DESC, hm.id DESC "
         "LIMIT "
      << limit;

  MYSQL_RES *res = DatabaseManager::getInstance().query(sql.str());
  std::vector<HealthMeasurement> list;
  if (!res)
    return list;

  MYSQL_ROW row;
  while ((row = mysql_fetch_row(res)) != nullptr) {
    HealthMeasurement m;
    m.id = row[0] ? std::stoi(row[0]) : 0;
    m.resident_id = row[1] ? std::stoi(row[1]) : 0;
    m.systolic = row[2] ? std::stoi(row[2]) : -1;
    m.diastolic = row[3] ? std::stoi(row[3]) : -1;
    m.blood_sugar = row[4] ? std::stod(row[4]) : -1.0;
    m.heart_rate = row[5] ? std::stoi(row[5]) : -1;
    m.height = row[6] ? std::stod(row[6]) : -1.0;
    m.weight = row[7] ? std::stod(row[7]) : -1.0;
    m.bmi = row[8] ? std::stod(row[8]) : -1.0;
    m.notes = safeStr(row[9]);
    m.measured_by = row[10] ? std::stoi(row[10]) : -1;
    m.measured_at = safeStr(row[11]);
    m.measured_by_name = safeStr(row[12]);
    list.push_back(m);
  }

  mysql_free_result(res);
  return list;
}

int HealthMeasurementDAO::insertMeasurement(const HealthMeasurement &m) {
  auto &db = DatabaseManager::getInstance();

  double bmi = -1.0;
  if (m.height > 0.0 && m.weight > 0.0) {
    double meter = m.height / 100.0;
    bmi = m.weight / (meter * meter);
  }

  std::ostringstream sql;
  sql << "INSERT INTO health_measurements "
         "(resident_id, systolic, diastolic, blood_sugar, heart_rate, height, "
         "weight, bmi, notes, measured_by, measured_at) VALUES ("
      << m.resident_id << ", " << sqlNullableInt(m.systolic) << ", "
      << sqlNullableInt(m.diastolic) << ", " << sqlNullableDouble(m.blood_sugar)
      << ", " << sqlNullableInt(m.heart_rate) << ", "
      << sqlNullableDouble(m.height) << ", " << sqlNullableDouble(m.weight)
      << ", " << sqlNullableDouble(bmi) << ", "
      << "'" << db.escape(m.notes) << "', "
      << ((m.measured_by > 0) ? std::to_string(m.measured_by) : "NULL") << ", ";

  if (m.measured_at.empty()) {
    sql << "CURRENT_TIMESTAMP";
  } else {
    sql << "'" << db.escape(m.measured_at) << "'";
  }

  sql << ")";

  db.execute(sql.str());
  const int new_id = static_cast<int>(db.lastInsertId());
  HealthMeasurement inserted = m;
  inserted.id = new_id;
  inserted.bmi = bmi;
  rebuildWarnings(db, new_id, inserted);
  return new_id;
}

HealthMeasurement HealthMeasurementDAO::getMeasurementById(int measurement_id) {
  std::ostringstream sql;
  sql << "SELECT hm.id, hm.resident_id, hm.systolic, hm.diastolic, "
         "hm.blood_sugar, hm.heart_rate, hm.height, hm.weight, hm.bmi, "
         "hm.notes, hm.measured_by, hm.measured_at, "
         "IFNULL(u.real_name, '') AS measured_by_name "
         "FROM health_measurements hm "
         "LEFT JOIN users u ON hm.measured_by = u.id "
         "WHERE hm.id = "
      << measurement_id << " LIMIT 1";

  MYSQL_RES *res = DatabaseManager::getInstance().query(sql.str());
  HealthMeasurement m;
  if (!res)
    return m;

  MYSQL_ROW row = mysql_fetch_row(res);
  if (row) {
    m.id = row[0] ? std::stoi(row[0]) : 0;
    m.resident_id = row[1] ? std::stoi(row[1]) : 0;
    m.systolic = row[2] ? std::stoi(row[2]) : -1;
    m.diastolic = row[3] ? std::stoi(row[3]) : -1;
    m.blood_sugar = row[4] ? std::stod(row[4]) : -1.0;
    m.heart_rate = row[5] ? std::stoi(row[5]) : -1;
    m.height = row[6] ? std::stod(row[6]) : -1.0;
    m.weight = row[7] ? std::stod(row[7]) : -1.0;
    m.bmi = row[8] ? std::stod(row[8]) : -1.0;
    m.notes = safeStr(row[9]);
    m.measured_by = row[10] ? std::stoi(row[10]) : -1;
    m.measured_at = safeStr(row[11]);
    m.measured_by_name = safeStr(row[12]);
  }

  mysql_free_result(res);
  return m;
}

bool HealthMeasurementDAO::updateMeasurement(int measurement_id,
                                             const HealthMeasurement &m) {
  if (measurement_id <= 0 || getMeasurementById(measurement_id).id == 0) {
    return false;
  }

  auto &db = DatabaseManager::getInstance();

  double bmi = -1.0;
  if (m.height > 0.0 && m.weight > 0.0) {
    const double meter = m.height / 100.0;
    bmi = m.weight / (meter * meter);
  }

  std::ostringstream sql;
  sql << "UPDATE health_measurements SET "
      << "resident_id = " << m.resident_id << ", "
      << "systolic = " << sqlNullableInt(m.systolic) << ", "
      << "diastolic = " << sqlNullableInt(m.diastolic) << ", "
      << "blood_sugar = " << sqlNullableDouble(m.blood_sugar) << ", "
      << "heart_rate = " << sqlNullableInt(m.heart_rate) << ", "
      << "height = " << sqlNullableDouble(m.height) << ", "
      << "weight = " << sqlNullableDouble(m.weight) << ", "
      << "bmi = " << sqlNullableDouble(bmi) << ", "
      << "notes = '" << db.escape(m.notes) << "', "
      << "measured_by = "
      << ((m.measured_by > 0) ? std::to_string(m.measured_by) : "NULL") << ", "
      << "measured_at = "
      << (m.measured_at.empty() ? "CURRENT_TIMESTAMP"
                                : ("'" + db.escape(m.measured_at) + "'"))
      << " WHERE id = " << measurement_id;

  db.execute(sql.str());

  HealthMeasurement updated = m;
  updated.id = measurement_id;
  updated.bmi = bmi;
  rebuildWarnings(db, measurement_id, updated);
  return true;
}

HealthRecord
HealthMeasurementDAO::getHealthRecordByResidentId(int resident_id) {
  std::string sql =
      "SELECT id, resident_id, blood_type, allergy_history, family_history, "
      "past_medical_history, created_at, updated_at "
      "FROM health_records WHERE resident_id = " +
      std::to_string(resident_id) + " LIMIT 1";

  MYSQL_RES *res = DatabaseManager::getInstance().query(sql);
  HealthRecord r;
  if (!res)
    return r;

  MYSQL_ROW row = mysql_fetch_row(res);
  if (row) {
    r.id = row[0] ? std::stoi(row[0]) : 0;
    r.resident_id = row[1] ? std::stoi(row[1]) : 0;
    r.blood_type = safeStr(row[2]);
    r.allergy_history = safeStr(row[3]);
    r.family_history = safeStr(row[4]);
    r.past_medical_history = safeStr(row[5]);
    r.created_at = safeStr(row[6]);
    r.updated_at = safeStr(row[7]);
  }

  mysql_free_result(res);
  return r;
}

bool HealthMeasurementDAO::upsertHealthRecordByResidentId(
    int resident_id, const HealthRecord &record) {
  auto &db = DatabaseManager::getInstance();
  const HealthRecord existing = getHealthRecordByResidentId(resident_id);
  std::ostringstream sql;

  if (existing.id == 0) {
    sql << "INSERT INTO health_records "
           "(resident_id, blood_type, allergy_history, family_history, "
           "past_medical_history) VALUES ("
        << resident_id << ", " << sqlQuotedOrNull(db, record.blood_type) << ", "
        << sqlQuotedOrNull(db, record.allergy_history) << ", "
        << sqlQuotedOrNull(db, record.family_history) << ", "
        << sqlQuotedOrNull(db, record.past_medical_history) << ")";
  } else {
    sql << "UPDATE health_records SET "
        << "blood_type = " << sqlQuotedOrNull(db, record.blood_type) << ", "
        << "allergy_history = " << sqlQuotedOrNull(db, record.allergy_history)
        << ", "
        << "family_history = " << sqlQuotedOrNull(db, record.family_history)
        << ", "
        << "past_medical_history = "
        << sqlQuotedOrNull(db, record.past_medical_history) << " "
        << "WHERE resident_id = " << resident_id;
  }

  db.execute(sql.str());
  return true;
}

int HealthMeasurementDAO::countWarnings() {
  std::string sql =
      "SELECT COUNT(*) FROM ("
      "SELECT hm.resident_id "
      "FROM health_measurements hm "
      "INNER JOIN ("
      "  SELECT resident_id, MAX(measured_at) AS latest_time "
      "  FROM health_measurements GROUP BY resident_id"
      ") latest ON hm.resident_id = latest.resident_id "
      "AND hm.measured_at = latest.latest_time "
      "WHERE (hm.systolic >= 140 OR hm.diastolic >= 90 "
      "   OR (hm.systolic IS NOT NULL AND (hm.systolic <= 90 OR hm.diastolic "
      "<= 60)) "
      "   OR hm.blood_sugar >= 7.0 "
      "   OR (hm.blood_sugar IS NOT NULL AND hm.blood_sugar <= 3.9) "
      "   OR (hm.heart_rate IS NOT NULL AND (hm.heart_rate > 100 OR "
      "hm.heart_rate < 60))) "
      "GROUP BY hm.resident_id"
      ") latest_warning_residents";
  MYSQL_RES *res = DatabaseManager::getInstance().query(sql);
  int count = 0;
  if (res) {
    MYSQL_ROW row = mysql_fetch_row(res);
    if (row && row[0]) {
      count = std::stoi(row[0]);
    }
    mysql_free_result(res);
  }
  return count;
}

bool HealthMeasurementDAO::residentExists(int resident_id) {
  std::string sql =
      "SELECT 1 FROM residents WHERE id = " + std::to_string(resident_id) +
      " LIMIT 1";
  MYSQL_RES *res = DatabaseManager::getInstance().query(sql);
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
  MYSQL_RES *res = DatabaseManager::getInstance().query(sql);
  bool exists = false;
  if (res) {
    exists = (mysql_fetch_row(res) != nullptr);
    mysql_free_result(res);
  }
  return exists;
}

std::vector<std::string>
HealthMeasurementDAO::getWarningMessagesByMeasurementId(int measurement_id) {
  std::string sql = "SELECT warning_type, warning_msg FROM health_warnings "
                    "WHERE measurement_id = " +
                    std::to_string(measurement_id) + " ORDER BY id ASC";

  MYSQL_RES *res = nullptr;
  try {
    res = DatabaseManager::getInstance().query(sql);
  } catch (...) {
    return {};
  }

  std::vector<std::string> warnings;
  if (!res)
    return warnings;

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
