#pragma once

#include <string>

#if __has_include("../include/json.hpp")
#  include "../include/json.hpp"
#elif __has_include("include/json.hpp")
#  include "include/json.hpp"
#else
#  include <nlohmann/json.hpp>
#endif

using json = nlohmann::json;

/**
 * @brief 慢性病管理业务层
 */
class DiseaseService {
public:
    json getDiseaseList();
    json addResidentDisease(int resident_id, const json& body);
    json getResidentDiseases(int resident_id);
};
