#include "DiseaseDAO.h"

#include "../utils/DatabaseManager.h"

namespace {

std::string safeStr(const char* s) {
    return (s != nullptr) ? std::string(s) : std::string();
}

enum DiseaseColumns {
    COL_DISEASE_ID = 0,
    COL_DISEASE_NAME,
    COL_DISEASE_CODE,
    COL_DISEASE_CATEGORY,
    COL_DISEASE_DESCRIPTION
};

enum ResidentDiseaseColumns {
    COL_RD_ID = 0,
    COL_RD_RESIDENT_ID,
    COL_RD_DISEASE_ID,
    COL_RD_DISEASE_NAME,
    COL_RD_DISEASE_CODE,
    COL_RD_CATEGORY,
    COL_RD_DIAGNOSED_DATE,
    COL_RD_STATUS,
    COL_RD_NOTES
};

} // namespace

std::vector<ChronicDisease> DiseaseDAO::listDiseases() {
    std::string sql =
        "SELECT id, disease_name, disease_code, category, description "
        "FROM chronic_diseases ORDER BY id ASC";

    MYSQL_RES* res = DatabaseManager::getInstance().query(sql);
    std::vector<ChronicDisease> list;
    if (!res) return list;

    MYSQL_ROW row;
    while ((row = mysql_fetch_row(res)) != nullptr) {
        ChronicDisease d;
        d.id = row[COL_DISEASE_ID] ? std::stoi(row[COL_DISEASE_ID]) : 0;
        d.disease_name = safeStr(row[COL_DISEASE_NAME]);
        d.disease_code = safeStr(row[COL_DISEASE_CODE]);
        d.category = safeStr(row[COL_DISEASE_CATEGORY]);
        d.description = safeStr(row[COL_DISEASE_DESCRIPTION]);
        list.push_back(d);
    }

    mysql_free_result(res);
    return list;
}

std::vector<ResidentDisease> DiseaseDAO::listResidentDiseases(int resident_id) {
    std::string sql =
        "SELECT rd.id, rd.resident_id, rd.disease_id, cd.disease_name, cd.disease_code, "
        "IFNULL(cd.category, ''), rd.diagnosed_date, IFNULL(rd.status, ''), IFNULL(rd.notes, '') "
        "FROM resident_diseases rd "
        "JOIN chronic_diseases cd ON rd.disease_id = cd.id "
        "WHERE rd.resident_id = " + std::to_string(resident_id) +
        " ORDER BY rd.id DESC";

    MYSQL_RES* res = DatabaseManager::getInstance().query(sql);
    std::vector<ResidentDisease> list;
    if (!res) return list;

    MYSQL_ROW row;
    while ((row = mysql_fetch_row(res)) != nullptr) {
        ResidentDisease d;
        d.id = row[COL_RD_ID] ? std::stoi(row[COL_RD_ID]) : 0;
        d.resident_id = row[COL_RD_RESIDENT_ID] ? std::stoi(row[COL_RD_RESIDENT_ID]) : 0;
        d.disease_id = row[COL_RD_DISEASE_ID] ? std::stoi(row[COL_RD_DISEASE_ID]) : 0;
        d.disease_name = safeStr(row[COL_RD_DISEASE_NAME]);
        d.disease_code = safeStr(row[COL_RD_DISEASE_CODE]);
        d.category = safeStr(row[COL_RD_CATEGORY]);
        d.diagnosed_date = safeStr(row[COL_RD_DIAGNOSED_DATE]);
        d.status = safeStr(row[COL_RD_STATUS]);
        d.notes = safeStr(row[COL_RD_NOTES]);
        list.push_back(d);
    }

    mysql_free_result(res);
    return list;
}

bool DiseaseDAO::residentExists(int resident_id) {
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

bool DiseaseDAO::diseaseExists(int disease_id) {
    std::string sql =
        "SELECT 1 FROM chronic_diseases WHERE id = " + std::to_string(disease_id) + " LIMIT 1";
    MYSQL_RES* res = DatabaseManager::getInstance().query(sql);
    bool exists = false;
    if (res) {
        exists = (mysql_fetch_row(res) != nullptr);
        mysql_free_result(res);
    }
    return exists;
}

bool DiseaseDAO::residentDiseaseExists(int resident_id, int disease_id) {
    std::string sql =
        "SELECT 1 FROM resident_diseases WHERE resident_id = " + std::to_string(resident_id) +
        " AND disease_id = " + std::to_string(disease_id) + " LIMIT 1";
    MYSQL_RES* res = DatabaseManager::getInstance().query(sql);
    bool exists = false;
    if (res) {
        exists = (mysql_fetch_row(res) != nullptr);
        mysql_free_result(res);
    }
    return exists;
}

int DiseaseDAO::addResidentDisease(int resident_id, int disease_id,
                                   const std::string& diagnosed_date,
                                   const std::string& status,
                                   const std::string& notes) {
    auto& db = DatabaseManager::getInstance();
    const std::string diagnosed = db.escape(diagnosed_date);
    const std::string status_safe = db.escape(status);
    const std::string notes_safe = db.escape(notes);

    std::string sql =
        "INSERT INTO resident_diseases (resident_id, disease_id, diagnosed_date, status, notes) VALUES (" +
        std::to_string(resident_id) + ", " + std::to_string(disease_id) + ", '" + diagnosed + "', '" +
        status_safe + "', '" + notes_safe + "')";

    db.execute(sql);
    return static_cast<int>(db.lastInsertId());
}
