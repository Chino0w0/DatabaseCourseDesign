#pragma once

#include <string>

/**
 * @brief 随访记录实体，对应数据库 visit_logs 表
 */
struct VisitLog {
    int         id{0};
    int         resident_id{0};
    int         visitor_user_id{0};
    std::string visitor_name;
    std::string visit_type;
    std::string visit_date;
    std::string content;
    std::string next_visit_date;
    std::string created_at;
};
