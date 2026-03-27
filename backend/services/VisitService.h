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
 * @brief 随访管理业务层
 */
class VisitService {
public:
    json getVisits(int resident_id);
    json createVisit(const json& body);
};
