#pragma once

#include <string>

// nlohmann/json：优先使用手动放置的头文件，否则使用 FetchContent 版本
#if __has_include("../include/json.hpp")
#  include "../include/json.hpp"
#elif __has_include("include/json.hpp")
#  include "include/json.hpp"
#else
#  include <nlohmann/json.hpp>
#endif

using json = nlohmann::json;

/**
 * @brief 居民档案业务逻辑层
 *
 * 负责参数校验、数据组装，并调用 ResidentDAO 完成数据库操作。
 * Controller 层通过本 Service 获取业务结果，不直接访问 DAO。
 */
class ResidentService {
public:
    // ============================================================
    // 社区接口
    // ============================================================

    /**
     * @brief 获取所有社区列表
     * @return 社区数组的 JSON 对象
     */
    json getCommunityList();

    /**
     * @brief 新增社区
     * @param body 请求体 JSON（name 必填；address / contact_phone 可选）
     * @return 包含新社区 ID 的 JSON 对象；name 缺失时返回含错误信息的对象
     */
    json createCommunity(const json& body);

    // ============================================================
    // 居民接口
    // ============================================================

    /**
     * @brief 分页查询居民列表
     * @param page         页码（从 1 开始）
     * @param size         每页数量
     * @param keyword      模糊搜索关键词（匹配姓名/身份证/手机）
     * @param community_id 按社区过滤；0 表示不过滤
     * @return 含 total / page / size / list 字段的 JSON 对象
     */
    json getResidentList(int page, int size,
                         const std::string& keyword,
                         int community_id);

    /**
     * @brief 获取单个居民详情
     * @param id 居民 ID
     * @return 居民 JSON 对象；不存在时返回 null
     */
    json getResidentDetail(int id);

    /**
     * @brief 新增居民
     * @param body 请求体 JSON（name / gender / id_card / birth_date 必填）
     * @return 含新居民 ID 的 JSON 对象；参数不合法时返回含 error 字段的对象
     */
    json createResident(const json& body);

    /**
     * @brief 更新居民信息
     * @param id   居民 ID
     * @param body 请求体 JSON
     * @return 更新成功返回 true；居民不存在返回 false；参数非法返回含 error 字段的对象
     */
    json updateResident(int id, const json& body);

    /**
     * @brief 删除居民
     * @param id 居民 ID
     * @return true 成功；false 居民不存在
     */
    bool deleteResident(int id);

private:
    /**
     * @brief 将 Resident 对象序列化为 JSON
     * @param r 居民实体
     * @return JSON 对象
     */
    json residentToJson(const struct Resident& r);

    /**
     * @brief 将 Community 对象序列化为 JSON
     * @param c 社区实体
     * @return JSON 对象
     */
    json communityToJson(const struct Community& c);
};
