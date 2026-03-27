#pragma once

#include <string>

/**
 * @brief 社区信息实体类，对应数据库 communities 表
 */
struct Community {
    int         id{0};             ///< 社区 ID（主键）
    std::string name;              ///< 社区名称
    std::string address;           ///< 社区地址
    std::string contact_phone;     ///< 联系电话
    std::string created_at;        ///< 创建时间
};
