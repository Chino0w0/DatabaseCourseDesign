#pragma once

#include <vector>
#include <string>

#include "../models/Resident.h"
#include "../models/Community.h"

/**
 * @brief 居民档案数据访问层
 *
 * 封装对 residents 表和 communities 表的所有 SQL 操作。
 * SQL 仅写在此 DAO 层；所有字符串参数均经过 DatabaseManager::escape()
 * 转义以防止 SQL 注入。
 */
class ResidentDAO {
public:
    // ============================================================
    // 社区（communities）相关操作
    // ============================================================

    /**
     * @brief 获取所有社区列表
     * @return Community 对象列表
     * @throws std::runtime_error 数据库操作失败时抛出
     */
    std::vector<Community> listCommunities();

    /**
     * @brief 按 ID 查询单个社区
     * @param id 社区 ID
     * @return Community 对象；若不存在则 id 字段为 0
     * @throws std::runtime_error 数据库操作失败时抛出
     */
    Community getCommunityById(int id);

    /**
     * @brief 新增社区
     * @param c 社区实体（id 字段忽略，由数据库自增）
     * @return 新插入记录的自增 ID
     * @throws std::runtime_error 数据库操作失败时抛出
     */
    int insertCommunity(const Community& c);

    /**
     * @brief 更新社区信息
     * @param id 社区 ID
     * @param c  包含新值的社区实体（id / created_at 字段忽略）
     * @return true 更新成功；false 社区不存在
     * @throws std::runtime_error 数据库操作失败时抛出
     */
    bool updateCommunity(int id, const Community& c);

    // ============================================================
    // 居民（residents）相关操作
    // ============================================================

    /**
     * @brief 统计满足条件的居民总数（用于分页）
     * @param keyword      模糊搜索关键词（匹配姓名/身份证号/手机号），空字符串表示不过滤
     * @param community_id 按社区过滤；0 表示不过滤
     * @return 满足条件的居民数量
     * @throws std::runtime_error 数据库操作失败时抛出
     */
    int countResidents(const std::string& keyword, int community_id);

    /**
     * @brief 分页查询居民列表（包含社区名称）
     * @param page         页码（从 1 开始）
     * @param size         每页数量
     * @param keyword      模糊搜索关键词，空字符串表示不过滤
     * @param community_id 按社区过滤；0 表示不过滤
     * @return Resident 对象列表（community_name 字段已从 communities 表 JOIN 填充）
     * @throws std::runtime_error 数据库操作失败时抛出
     */
    std::vector<Resident> listResidents(int page, int size,
                                        const std::string& keyword,
                                        int community_id);

    /**
     * @brief 按 ID 查询单个居民（包含社区名称）
     * @param id 居民 ID
     * @return Resident 对象；若不存在则 id 字段为 0
     * @throws std::runtime_error 数据库操作失败时抛出
     */
    Resident getResidentById(int id);

    /**
     * @brief 新增居民
     * @param r 居民实体（id / created_at / updated_at 字段由数据库填充）
     * @return 新插入记录的自增 ID
     * @throws std::runtime_error 数据库操作失败时抛出
     */
    int insertResident(const Resident& r);

    /**
     * @brief 更新居民信息
     * @param id 居民 ID
     * @param r  包含新值的居民实体（id / created_at 字段忽略）
     * @return true 更新成功；false 居民不存在
     * @throws std::runtime_error 数据库操作失败时抛出
     */
    bool updateResident(int id, const Resident& r);

    /**
     * @brief 删除居民
     * @param id 居民 ID
     * @return true 删除成功；false 居民不存在
     * @throws std::runtime_error 数据库操作失败时抛出
     */
    bool deleteResident(int id);

    /**
     * @brief 检查居民是否存在
     * @param id 居民 ID
     * @return true 存在；false 不存在
     * @throws std::runtime_error 数据库操作失败时抛出
     */
    bool residentExists(int id);

    /**
     * @brief 检查身份证号是否已被其他居民使用
     * @param id_card   待检查的身份证号
     * @param exclude_id 排除的居民 ID（更新时传入自身 ID，新增时传 0）
     * @return true 已存在；false 不存在
     * @throws std::runtime_error 数据库操作失败时抛出
     */
    bool idCardExists(const std::string& id_card, int exclude_id = 0);
};
