#pragma once

#include <vector>

#include "../models/VisitLog.h"

/**
 * @brief 随访记录数据访问层
 */
class VisitLogDAO {
public:
    std::vector<VisitLog> listByResidentId(int resident_id);
    int insertVisitLog(const VisitLog& log);

    bool residentExists(int resident_id);
    bool userExists(int user_id);
};
