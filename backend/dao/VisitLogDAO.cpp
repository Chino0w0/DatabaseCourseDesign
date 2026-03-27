#include "VisitLogDAO.h"

#include "../utils/DatabaseManager.h"

namespace {

std::string safeStr(const char* s) {
    return (s != nullptr) ? std::string(s) : std::string();
}

} // namespace

std::vector<VisitLog> VisitLogDAO::listByResidentId(int resident_id) {
    std::string sql =
        "SELECT vl.id, vl.resident_id, vl.visitor_user_id, IFNULL(u.real_name, ''), "
        "IFNULL(vl.visit_type, ''), vl.visit_date, IFNULL(vl.content, ''), "
        "vl.next_visit_date, vl.created_at "
        "FROM visit_logs vl "
        "LEFT JOIN users u ON vl.visitor_user_id = u.id "
        "WHERE vl.resident_id = " + std::to_string(resident_id) +
        " ORDER BY vl.visit_date DESC, vl.id DESC";

    MYSQL_RES* res = DatabaseManager::getInstance().query(sql);
    std::vector<VisitLog> list;
    if (!res) return list;

    MYSQL_ROW row;
    while ((row = mysql_fetch_row(res)) != nullptr) {
        VisitLog log;
        log.id = row[0] ? std::stoi(row[0]) : 0;
        log.resident_id = row[1] ? std::stoi(row[1]) : 0;
        log.visitor_user_id = row[2] ? std::stoi(row[2]) : 0;
        log.visitor_name = safeStr(row[3]);
        log.visit_type = safeStr(row[4]);
        log.visit_date = safeStr(row[5]);
        log.content = safeStr(row[6]);
        log.next_visit_date = safeStr(row[7]);
        log.created_at = safeStr(row[8]);
        list.push_back(log);
    }

    mysql_free_result(res);
    return list;
}

int VisitLogDAO::insertVisitLog(const VisitLog& log) {
    auto& db = DatabaseManager::getInstance();

    const std::string visit_type = db.escape(log.visit_type);
    const std::string visit_date = db.escape(log.visit_date);
    const std::string content = db.escape(log.content);

    std::string sql =
        "INSERT INTO visit_logs (resident_id, visitor_user_id, visit_type, visit_date, content, next_visit_date) VALUES (" +
        std::to_string(log.resident_id) + ", " + std::to_string(log.visitor_user_id) + ", '" +
        visit_type + "', '" + visit_date + "', '" + content + "', ";

    if (log.next_visit_date.empty()) {
        sql += "NULL";
    } else {
        sql += "'" + db.escape(log.next_visit_date) + "'";
    }

    sql += ")";

    db.execute(sql);
    return static_cast<int>(db.lastInsertId());
}

bool VisitLogDAO::residentExists(int resident_id) {
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

bool VisitLogDAO::userExists(int user_id) {
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
