#include "ResidentDAO.h"

#include "../utils/DatabaseManager.h"

#include <sstream>
#include <stdexcept>

// ============================================================
// 内部辅助：安全读取 MYSQL_ROW 中的字段值（NULL 时返回空字符串）
// ============================================================
static std::string safeStr(const char* s) {
    return (s != nullptr) ? std::string(s) : std::string();
}

// ============================================================
// 社区（communities）操作
// ============================================================

std::vector<Community> ResidentDAO::listCommunities() {
    std::string sql =
        "SELECT id, name, address, contact_phone, created_at "
        "FROM communities ORDER BY id ASC";

    MYSQL_RES* res = DatabaseManager::getInstance().query(sql);
    std::vector<Community> list;

    if (!res) return list;

    MYSQL_ROW row;
    while ((row = mysql_fetch_row(res)) != nullptr) {
        Community c;
        c.id            = row[0] ? std::stoi(row[0]) : 0;
        c.name          = safeStr(row[1]);
        c.address       = safeStr(row[2]);
        c.contact_phone = safeStr(row[3]);
        c.created_at    = safeStr(row[4]);
        list.push_back(c);
    }

    mysql_free_result(res);
    return list;
}

Community ResidentDAO::getCommunityById(int id) {
    std::string sql =
        "SELECT id, name, address, contact_phone, created_at "
        "FROM communities WHERE id = " + std::to_string(id);

    MYSQL_RES* res = DatabaseManager::getInstance().query(sql);
    Community c;

    if (!res) return c;

    MYSQL_ROW row = mysql_fetch_row(res);
    if (row) {
        c.id            = row[0] ? std::stoi(row[0]) : 0;
        c.name          = safeStr(row[1]);
        c.address       = safeStr(row[2]);
        c.contact_phone = safeStr(row[3]);
        c.created_at    = safeStr(row[4]);
    }

    mysql_free_result(res);
    return c;
}

int ResidentDAO::insertCommunity(const Community& c) {
    auto& db = DatabaseManager::getInstance();

    std::string name    = db.escape(c.name);
    std::string addr    = db.escape(c.address);
    std::string phone   = db.escape(c.contact_phone);

    std::string sql =
        "INSERT INTO communities (name, address, contact_phone) "
        "VALUES ('" + name + "', '" + addr + "', '" + phone + "')";

    db.execute(sql);
    return static_cast<int>(db.lastInsertId());
}

bool ResidentDAO::updateCommunity(int id, const Community& c) {
    if (getCommunityById(id).id == 0) return false;

    auto& db = DatabaseManager::getInstance();
    std::string name  = db.escape(c.name);
    std::string addr  = db.escape(c.address);
    std::string phone = db.escape(c.contact_phone);

    std::ostringstream sql;
    sql << "UPDATE communities SET "
        << "name = '" << name << "', "
        << "address = '" << addr << "', "
        << "contact_phone = '" << phone << "' "
        << "WHERE id = " << id;

    db.execute(sql.str());
    return true;
}

// ============================================================
// 居民（residents）操作
// ============================================================

int ResidentDAO::countResidents(const std::string& keyword, int community_id) {
    auto& db = DatabaseManager::getInstance();

    std::ostringstream sql;
    sql << "SELECT COUNT(*) FROM residents r WHERE 1=1";

    if (!keyword.empty()) {
        std::string kw = db.escape(keyword);
        sql << " AND (r.name LIKE '%" << kw << "%'"
            << " OR r.id_card LIKE '%" << kw << "%'"
            << " OR r.phone LIKE '%" << kw << "%')";
    }

    if (community_id > 0) {
        sql << " AND r.community_id = " << community_id;
    }

    MYSQL_RES* res = db.query(sql.str());
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

std::vector<Resident> ResidentDAO::listResidents(int page, int size,
                                                  const std::string& keyword,
                                                  int community_id) {
    auto& db = DatabaseManager::getInstance();

    if (page < 1) page = 1;
    if (size < 1) size = 10;
    int offset = (page - 1) * size;

    std::ostringstream sql;
    sql << "SELECT r.id, r.name, r.gender, r.id_card, r.birth_date, "
           "r.phone, r.community_id, IFNULL(c.name,'') AS community_name, "
           "r.address, r.emergency_contact, r.emergency_phone, "
           "r.created_at, r.updated_at "
           "FROM residents r "
           "LEFT JOIN communities c ON r.community_id = c.id "
           "WHERE 1=1";

    if (!keyword.empty()) {
        std::string kw = db.escape(keyword);
        sql << " AND (r.name LIKE '%" << kw << "%'"
            << " OR r.id_card LIKE '%" << kw << "%'"
            << " OR r.phone LIKE '%" << kw << "%')";
    }

    if (community_id > 0) {
        sql << " AND r.community_id = " << community_id;
    }

    sql << " ORDER BY r.id DESC"
        << " LIMIT " << size << " OFFSET " << offset;

    MYSQL_RES* res = db.query(sql.str());
    std::vector<Resident> list;

    if (!res) return list;

    MYSQL_ROW row;
    while ((row = mysql_fetch_row(res)) != nullptr) {
        Resident r;
        r.id                = row[0]  ? std::stoi(row[0])  : 0;
        r.name              = safeStr(row[1]);
        r.gender            = safeStr(row[2]);
        r.id_card           = safeStr(row[3]);
        r.birth_date        = safeStr(row[4]);
        r.phone             = safeStr(row[5]);
        r.community_id      = row[6]  ? std::stoi(row[6])  : 0;
        r.community_name    = safeStr(row[7]);
        r.address           = safeStr(row[8]);
        r.emergency_contact = safeStr(row[9]);
        r.emergency_phone   = safeStr(row[10]);
        r.created_at        = safeStr(row[11]);
        r.updated_at        = safeStr(row[12]);
        list.push_back(r);
    }

    mysql_free_result(res);
    return list;
}

Resident ResidentDAO::getResidentById(int id) {
    std::string sql =
        "SELECT r.id, r.name, r.gender, r.id_card, r.birth_date, "
        "r.phone, r.community_id, IFNULL(c.name,'') AS community_name, "
        "r.address, r.emergency_contact, r.emergency_phone, "
        "r.created_at, r.updated_at "
        "FROM residents r "
        "LEFT JOIN communities c ON r.community_id = c.id "
        "WHERE r.id = " + std::to_string(id);

    MYSQL_RES* res = DatabaseManager::getInstance().query(sql);
    Resident r;

    if (!res) return r;

    MYSQL_ROW row = mysql_fetch_row(res);
    if (row) {
        r.id                = row[0]  ? std::stoi(row[0])  : 0;
        r.name              = safeStr(row[1]);
        r.gender            = safeStr(row[2]);
        r.id_card           = safeStr(row[3]);
        r.birth_date        = safeStr(row[4]);
        r.phone             = safeStr(row[5]);
        r.community_id      = row[6]  ? std::stoi(row[6])  : 0;
        r.community_name    = safeStr(row[7]);
        r.address           = safeStr(row[8]);
        r.emergency_contact = safeStr(row[9]);
        r.emergency_phone   = safeStr(row[10]);
        r.created_at        = safeStr(row[11]);
        r.updated_at        = safeStr(row[12]);
    }

    mysql_free_result(res);
    return r;
}

int ResidentDAO::insertResident(const Resident& r) {
    auto& db = DatabaseManager::getInstance();

    std::string name      = db.escape(r.name);
    std::string gender    = db.escape(r.gender);
    std::string id_card   = db.escape(r.id_card);
    std::string birth     = db.escape(r.birth_date);
    std::string phone     = db.escape(r.phone);
    std::string address   = db.escape(r.address);
    std::string ec        = db.escape(r.emergency_contact);
    std::string ep        = db.escape(r.emergency_phone);

    std::ostringstream sql;
    sql << "INSERT INTO residents "
           "(name, gender, id_card, birth_date, phone, community_id, "
           " address, emergency_contact, emergency_phone) VALUES ("
        << "'" << name   << "', "
        << "'" << gender << "', "
        << "'" << id_card << "', "
        << "'" << birth  << "', "
        << "'" << phone  << "', ";

    if (r.community_id > 0) {
        sql << r.community_id;
    } else {
        sql << "NULL";
    }

    sql << ", '" << address << "', '"
        << ec << "', '"
        << ep << "')";

    db.execute(sql.str());
    return static_cast<int>(db.lastInsertId());
}

bool ResidentDAO::updateResident(int id, const Resident& r) {
    if (!residentExists(id)) return false;

    auto& db = DatabaseManager::getInstance();

    std::string name      = db.escape(r.name);
    std::string gender    = db.escape(r.gender);
    std::string id_card   = db.escape(r.id_card);
    std::string birth     = db.escape(r.birth_date);
    std::string phone     = db.escape(r.phone);
    std::string address   = db.escape(r.address);
    std::string ec        = db.escape(r.emergency_contact);
    std::string ep        = db.escape(r.emergency_phone);

    std::ostringstream sql;
    sql << "UPDATE residents SET "
        << "name = '"              << name    << "', "
        << "gender = '"            << gender  << "', "
        << "id_card = '"           << id_card << "', "
        << "birth_date = '"        << birth   << "', "
        << "phone = '"             << phone   << "', "
        << "community_id = ";

    if (r.community_id > 0) {
        sql << r.community_id;
    } else {
        sql << "NULL";
    }

    sql << ", address = '"           << address << "', "
        << "emergency_contact = '"   << ec      << "', "
        << "emergency_phone = '"     << ep      << "' "
        << "WHERE id = " << id;

    db.execute(sql.str());
    return true;
}

bool ResidentDAO::deleteResident(int id) {
    if (!residentExists(id)) return false;

    std::string sql = "DELETE FROM residents WHERE id = " + std::to_string(id);
    DatabaseManager::getInstance().execute(sql);
    return true;
}

bool ResidentDAO::residentExists(int id) {
    std::string sql =
        "SELECT 1 FROM residents WHERE id = " + std::to_string(id) + " LIMIT 1";

    MYSQL_RES* res = DatabaseManager::getInstance().query(sql);
    bool exists = false;

    if (res) {
        exists = (mysql_fetch_row(res) != nullptr);
        mysql_free_result(res);
    }

    return exists;
}

bool ResidentDAO::idCardExists(const std::string& id_card, int exclude_id) {
    auto& db = DatabaseManager::getInstance();
    std::string card = db.escape(id_card);

    std::string sql =
        "SELECT 1 FROM residents WHERE id_card = '" + card + "'";

    if (exclude_id > 0) {
        sql += " AND id <> " + std::to_string(exclude_id);
    }

    sql += " LIMIT 1";

    MYSQL_RES* res = db.query(sql);
    bool exists = false;

    if (res) {
        exists = (mysql_fetch_row(res) != nullptr);
        mysql_free_result(res);
    }

    return exists;
}
