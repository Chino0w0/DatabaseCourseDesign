#pragma once

#include <string>

/**
 * @brief 慢性病字典实体，对应数据库 chronic_diseases 表
 */
struct ChronicDisease {
    int         id{0};
    std::string disease_name;
    std::string disease_code;
    std::string category;
    std::string description;
};
