#pragma once

#include <string>

/**
 * @brief 居民基本信息实体类，对应数据库 residents 表
 */
struct Resident {
    int         id{0};                  ///< 居民 ID（主键）
    std::string name;                   ///< 姓名
    std::string gender;                 ///< 性别（男/女）
    std::string id_card;                ///< 身份证号
    std::string birth_date;             ///< 出生日期（YYYY-MM-DD）
    std::string phone;                  ///< 联系电话
    int         community_id{0};        ///< 所属社区 ID
    std::string community_name;         ///< 所属社区名称（JOIN 填充，非数据库字段）
    std::string address;                ///< 详细地址
    std::string emergency_contact;      ///< 紧急联系人
    std::string emergency_phone;        ///< 紧急联系人电话
    std::string created_at;             ///< 建档时间
    std::string updated_at;             ///< 更新时间
};
