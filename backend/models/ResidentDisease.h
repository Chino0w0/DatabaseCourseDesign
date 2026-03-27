#pragma once

#include <string>

/**
 * @brief 居民慢性病关联实体（含疾病字典信息）
 */
struct ResidentDisease {
    int         id{0};
    int         resident_id{0};
    int         disease_id{0};
    std::string disease_name;
    std::string disease_code;
    std::string category;
    std::string diagnosed_date;
    std::string status;
    std::string notes;
};
