cmake_minimum_required(VERSION 3.14)
project(community_health_server CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ============================================================
# 第三方依赖：通过 FetchContent 自动下载（若 include/ 目录中
# 已手动放置头文件，CMake 也会通过 target_include_directories
# 正确找到它们）
# ============================================================
include(FetchContent)

# cpp-httplib — 轻量级 C++ HTTP 服务器/客户端单头文件库
FetchContent_Declare(
    httplib
    GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
    GIT_TAG        v0.15.3
)

# nlohmann/json — 现代 C++ JSON 解析单头文件库
FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG        v3.11.3
)

FetchContent_MakeAvailable(httplib nlohmann_json)

# ============================================================
# 查找 MySQL 客户端库（libmysqlclient）
# Ubuntu/Debian: sudo apt install libmysqlclient-dev
# CentOS/RHEL:   sudo yum install mysql-devel
# ============================================================
find_package(PkgConfig REQUIRED)
pkg_check_modules(MYSQLCLIENT REQUIRED mysqlclient)

# ============================================================
# 收集全部源文件
# ============================================================
file(GLOB_RECURSE SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/utils/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/dao/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/services/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/controllers/*.cpp"
)

# ============================================================
# 构建可执行文件
# ============================================================
add_executable(community_health_server ${SOURCES})

target_include_directories(community_health_server PRIVATE
    # 手动放置的第三方头文件（优先级最高）
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    # MySQL 头文件
    ${MYSQLCLIENT_INCLUDE_DIRS}
)

target_link_libraries(community_health_server PRIVATE
    httplib::httplib
    nlohmann_json::nlohmann_json
    ${MYSQLCLIENT_LIBRARIES}
)

target_compile_options(community_health_server PRIVATE
    ${MYSQLCLIENT_CFLAGS_OTHER}
    -Wall
    -Wextra
    -Wno-unused-parameter
)

# ============================================================
# 测试目标（CTest 集成）
# ============================================================
enable_testing()

add_executable(community_health_tests
    tests/test_all.cpp
    utils/DatabaseManager.cpp
)

target_include_directories(community_health_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${MYSQLCLIENT_INCLUDE_DIRS}
)

target_link_libraries(community_health_tests PRIVATE
    httplib::httplib
    nlohmann_json::nlohmann_json
    ${MYSQLCLIENT_LIBRARIES}
)

target_compile_options(community_health_tests PRIVATE
    ${MYSQLCLIENT_CFLAGS_OTHER}
    -Wall
    -Wextra
    -Wno-unused-parameter
)

# 注册到 CTest
add_test(NAME BackendInfraTests COMMAND community_health_tests)

# 透传数据库连接主机/端口/库名等非敏感配置。
# TEST_DB_PASS（数据库密码）必须在运行测试前通过外部环境变量提供，
# 例如：export TEST_DB_PASS=<your_password> && ctest
# 密码请参照 sql/permissions.sql 中 ch_admin 账户的设置。
set_tests_properties(BackendInfraTests PROPERTIES
    ENVIRONMENT "TEST_DB_HOST=127.0.0.1;TEST_DB_PORT=3306;TEST_DB_USER=ch_admin;TEST_DB_NAME=community_health"
)

# ============================================================
# 构建说明（cmake --build build 后生成 build/community_health_server）
# ============================================================
message(STATUS "构建目标: community_health_server")
message(STATUS "测试目标: community_health_tests")
message(STATUS "C++ 标准: C++${CMAKE_CXX_STANDARD}")
