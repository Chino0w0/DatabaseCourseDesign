#pragma once

#include <string>
#include <vector>

#include "../models/ChronicDisease.h"
#include "../models/ResidentDisease.h"

/**
 * @brief 慢性病管理数据访问层
 */
class DiseaseDAO {
public:
    std::vector<ChronicDisease> listDiseases();
    std::vector<ResidentDisease> listResidentDiseases(int resident_id);

    bool residentExists(int resident_id);
    bool diseaseExists(int disease_id);
    bool residentDiseaseExists(int resident_id, int disease_id);

    int addResidentDisease(int resident_id, int disease_id,
                           const std::string& diagnosed_date,
                           const std::string& status,
                           const std::string& notes);
};
