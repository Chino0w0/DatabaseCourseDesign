cmake_minimum_required(VERSION 3.14)
project(community_health_server CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ============================================================
# 第三方依赖：优先使用 include/ 目录中的本地单头文件；
# 仅当本地文件缺失时，才回退到 FetchContent 自动下载。
# 这样 start_linux.sh / make build 在离线环境下也不会因为
# 缺少 git 或无法访问 GitHub 而失败。
# ============================================================
set(LOCAL_HTTPLIB_HEADER ${CMAKE_CURRENT_SOURCE_DIR}/include/httplib.h)
set(LOCAL_JSON_HEADER ${CMAKE_CURRENT_SOURCE_DIR}/include/json.hpp)

if(EXISTS ${LOCAL_HTTPLIB_HEADER})
    add_library(httplib_local INTERFACE)
    add_library(httplib::httplib ALIAS httplib_local)
    target_include_directories(httplib_local INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    )
    target_link_libraries(httplib_local INTERFACE
        $<$<PLATFORM_ID:Windows>:ws2_32>
        $<$<PLATFORM_ID:Windows>:crypt32>
    )
    message(STATUS "使用本地 httplib 头文件: ${LOCAL_HTTPLIB_HEADER}")
else()
    include(FetchContent)
    FetchContent_Declare(
        httplib
        GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
        GIT_TAG        v0.15.3
    )
    FetchContent_MakeAvailable(httplib)
    message(STATUS "本地未找到 httplib.h，回退到 FetchContent 下载 cpp-httplib")
endif()

if(EXISTS ${LOCAL_JSON_HEADER})
    add_library(nlohmann_json_local INTERFACE)
    add_library(nlohmann_json::nlohmann_json ALIAS nlohmann_json_local)
    target_include_directories(nlohmann_json_local INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    )
    message(STATUS "使用本地 json.hpp 头文件: ${LOCAL_JSON_HEADER}")
else()
    include(FetchContent)
    FetchContent_Declare(
        nlohmann_json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG        v3.11.3
    )
    FetchContent_MakeAvailable(nlohmann_json)
    message(STATUS "本地未找到 json.hpp，回退到 FetchContent 下载 nlohmann/json")
endif()

# ============================================================
# 查找 MySQL / MariaDB 客户端开发库
# 优先走 pkg-config；若系统未提供 pkg-config，则回退到 find_path/find_library。
# Ubuntu/Debian 可安装：default-libmysqlclient-dev 或 libmariadb-dev
# Windows + MSYS2/UCRT64 下如果存在静态客户端库，则优先用于发布构建，
# 以尽量减少用户机器上额外缺失的 DLL。
# ============================================================
set(MYSQLCLIENT_INCLUDE_DIRS "")
set(MYSQLCLIENT_LIBRARIES "")
set(MYSQLCLIENT_CFLAGS_OTHER "")
set(MYSQLCLIENT_SYSTEM_LIBRARIES "")
set(MYSQLCLIENT_STATIC_DEPENDENCY_LIBRARIES "")

# 方案 B：Windows 下默认仅静态链接 libgcc/libstdc++ 运行时，
# 数据库客户端库保持动态链接。这样比“全静态 + libmariadbclient.a”
# 更稳定，避免额外手工补 zlib/zstd/curl 等静态依赖链。
set(CH_WINDOWS_STATIC_RUNTIME_DEFAULT OFF)
if(WIN32 AND MINGW)
    set(CH_WINDOWS_STATIC_RUNTIME_DEFAULT ON)
endif()
option(CH_WINDOWS_STATIC_RUNTIME
    "On Windows MinGW builds, prefer static libgcc/libstdc++ runtime"
    ${CH_WINDOWS_STATIC_RUNTIME_DEFAULT}
)
option(CH_WINDOWS_STATIC_DBCLIENT
    "On Windows MinGW builds, prefer static MariaDB/MySQL client libraries"
    OFF
)

find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
    pkg_check_modules(MYSQLCLIENT QUIET mysqlclient)
    if(NOT MYSQLCLIENT_FOUND)
        pkg_check_modules(MYSQLCLIENT QUIET libmariadb)
    endif()
endif()

if(NOT MYSQLCLIENT_FOUND)
    find_path(MYSQLCLIENT_INCLUDE_DIR
        NAMES mysql/mysql.h mariadb/mysql.h
        PATH_SUFFIXES include include/mysql include/mariadb mysql mariadb
    )

    find_library(MYSQLCLIENT_LIBRARY
        NAMES mysqlclient mariadb libmysqlclient libmariadb
    )

    if(MYSQLCLIENT_INCLUDE_DIR AND MYSQLCLIENT_LIBRARY)
        set(MYSQLCLIENT_FOUND TRUE)
        set(MYSQLCLIENT_INCLUDE_DIRS ${MYSQLCLIENT_INCLUDE_DIR})
        set(MYSQLCLIENT_LIBRARIES ${MYSQLCLIENT_LIBRARY})
    endif()
endif()

if(NOT MYSQLCLIENT_FOUND)
    message(FATAL_ERROR
        "未找到 MySQL/MariaDB 客户端开发库。请安装 default-libmysqlclient-dev 或 libmariadb-dev。")
endif()

# 某些环境下 pkg_check_modules 只填充了 pkgcfg_lib_*，导致
# MYSQLCLIENT_LIBRARIES 为空，进而链接阶段缺少 -lmysqlclient。
# 这里做一次兜底，保证可执行文件稳定链接。
if(MYSQLCLIENT_FOUND AND "${MYSQLCLIENT_LIBRARIES}" STREQUAL "")
    if(DEFINED pkgcfg_lib_MYSQLCLIENT_mysqlclient)
        set(MYSQLCLIENT_LIBRARIES ${pkgcfg_lib_MYSQLCLIENT_mysqlclient})
    elseif(DEFINED pkgcfg_lib_MYSQLCLIENT_libmariadb)
        set(MYSQLCLIENT_LIBRARIES ${pkgcfg_lib_MYSQLCLIENT_libmariadb})
    endif()
endif()

if(WIN32 AND MINGW)
    list(APPEND MYSQLCLIENT_SYSTEM_LIBRARIES
        ws2_32
        crypt32
        secur32
        iphlpapi
        bcrypt
        ncrypt
        shlwapi
    )
endif()

if(WIN32 AND MINGW AND CH_WINDOWS_STATIC_DBCLIENT)
    set(_MYSQLCLIENT_PREFERRED_LIBRARIES "")
    set(_MYSQLCLIENT_SEARCH_DIRS
        ${MYSQLCLIENT_LIBRARY_DIRS}
        ${MYSQLCLIENT_STATIC_LIBRARY_DIRS}
        ${MYSQLCLIENT_LIBDIR}
        C:/msys64/ucrt64/lib
    )

    foreach(_lib IN LISTS MYSQLCLIENT_LIBRARIES)
        set(_preferred_lib "${_lib}")

        if(IS_ABSOLUTE "${_lib}")
            get_filename_component(_lib_name "${_lib}" NAME)
            get_filename_component(_lib_dir "${_lib}" DIRECTORY)

            if(_lib_name STREQUAL "libmariadb.dll.a" AND EXISTS "${_lib_dir}/libmariadbclient.a")
                set(_preferred_lib "${_lib_dir}/libmariadbclient.a")
            elseif(_lib_name STREQUAL "libmysqlclient.dll.a" AND EXISTS "${_lib_dir}/libmysqlclient.a")
                set(_preferred_lib "${_lib_dir}/libmysqlclient.a")
            endif()
        else()
            if(_lib STREQUAL "mariadb" OR _lib STREQUAL "libmariadb")
                find_library(_MYSQLCLIENT_STATIC_MATCH
                    NAMES mariadbclient libmariadbclient mariadb libmariadb
                    PATHS ${_MYSQLCLIENT_SEARCH_DIRS}
                    NO_DEFAULT_PATH
                )
                if(_MYSQLCLIENT_STATIC_MATCH)
                    set(_preferred_lib "${_MYSQLCLIENT_STATIC_MATCH}")
                endif()
            elseif(_lib STREQUAL "mysqlclient" OR _lib STREQUAL "libmysqlclient")
                find_library(_MYSQLCLIENT_STATIC_MATCH
                    NAMES mysqlclient libmysqlclient mariadbclient libmariadbclient
                    PATHS ${_MYSQLCLIENT_SEARCH_DIRS}
                    NO_DEFAULT_PATH
                )
                if(_MYSQLCLIENT_STATIC_MATCH)
                    set(_preferred_lib "${_MYSQLCLIENT_STATIC_MATCH}")
                endif()
            endif()
        endif()

        list(APPEND _MYSQLCLIENT_PREFERRED_LIBRARIES "${_preferred_lib}")
    endforeach()

    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(_MSYS2_UCRT64_LIB_DIR "C:/msys64/ucrt64/lib")
        if(EXISTS "${_MSYS2_UCRT64_LIB_DIR}/libz.a")
            list(APPEND MYSQLCLIENT_STATIC_DEPENDENCY_LIBRARIES "${_MSYS2_UCRT64_LIB_DIR}/libz.a")
        endif()
        if(EXISTS "${_MSYS2_UCRT64_LIB_DIR}/libzstd.a")
            list(APPEND MYSQLCLIENT_STATIC_DEPENDENCY_LIBRARIES "${_MSYS2_UCRT64_LIB_DIR}/libzstd.a")
        endif()
        if(EXISTS "${_MSYS2_UCRT64_LIB_DIR}/libcurl.a")
            list(APPEND MYSQLCLIENT_STATIC_DEPENDENCY_LIBRARIES "${_MSYS2_UCRT64_LIB_DIR}/libcurl.a")
        endif()
    endif()

    if(NOT "${_MYSQLCLIENT_PREFERRED_LIBRARIES}" STREQUAL "")
        set(MYSQLCLIENT_LIBRARIES ${_MYSQLCLIENT_PREFERRED_LIBRARIES})
        message(STATUS "Windows/MinGW 构建：优先使用静态 MySQL/MariaDB 客户端库")
    else()
        find_library(MYSQLCLIENT_STATIC_PREFERRED_LIBRARY
            NAMES mariadbclient mysqlclient libmariadbclient libmysqlclient
            PATHS ${_MYSQLCLIENT_SEARCH_DIRS}
            NO_DEFAULT_PATH
        )
        if(MYSQLCLIENT_STATIC_PREFERRED_LIBRARY)
            set(MYSQLCLIENT_LIBRARIES ${MYSQLCLIENT_STATIC_PREFERRED_LIBRARY})
            message(STATUS "Windows/MinGW 构建：使用静态 MySQL/MariaDB 客户端库: ${MYSQLCLIENT_STATIC_PREFERRED_LIBRARY}")
        endif()
    endif()
else()
    message(STATUS "Windows/MinGW 构建：使用方案 B（静态运行时 + 动态数据库客户端库）")
endif()

# ============================================================
# 收集全部源文件
# 注意：使用 GLOB（非递归）+ 明确列出 main.cpp，避免 GLOB_RECURSE
# 将构建目录（build/_deps/...）中的同名文件误收进来。
# ============================================================
file(GLOB SOURCES
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
    # 项目源码根目录（供 controllers/dao/services 跨目录引用头文件）
    ${CMAKE_CURRENT_SOURCE_DIR}
    # 手动放置的第三方头文件（优先级最高）
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    # MySQL 头文件
    ${MYSQLCLIENT_INCLUDE_DIRS}
)

target_link_libraries(community_health_server PRIVATE
    httplib::httplib
    nlohmann_json::nlohmann_json
    ${MYSQLCLIENT_LIBRARIES}
    ${MYSQLCLIENT_STATIC_DEPENDENCY_LIBRARIES}
    ${MYSQLCLIENT_SYSTEM_LIBRARIES}
)

target_compile_options(community_health_server PRIVATE
    ${MYSQLCLIENT_CFLAGS_OTHER}
    -Wall
    -Wextra
    -Wno-unused-parameter
)

if(WIN32 AND MINGW AND CH_WINDOWS_STATIC_RUNTIME)
    # 方案 B 下只静态链接 libgcc/libstdc++，不要使用全局 -static。
    # 否则链接器会拒绝使用 libmariadb.dll.a 这类导入库，导致
    # "cannot find -lmariadb" 之类错误。
    target_link_options(community_health_server PRIVATE
        -static-libgcc
        -static-libstdc++
    )
endif()

# ============================================================
# 测试目标（CTest 集成）
# ============================================================
enable_testing()

add_executable(community_health_tests
    tests/test_all.cpp
    utils/DatabaseManager.cpp
    services/DiseaseService.cpp
    services/VisitService.cpp
    dao/DiseaseDAO.cpp
    dao/VisitLogDAO.cpp
    dao/UserDAO.cpp
    services/AuthService.cpp
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
    ${MYSQLCLIENT_STATIC_DEPENDENCY_LIBRARIES}
    ${MYSQLCLIENT_SYSTEM_LIBRARIES}
)

target_compile_options(community_health_tests PRIVATE
    ${MYSQLCLIENT_CFLAGS_OTHER}
    -Wall
    -Wextra
    -Wno-unused-parameter
)

if(WIN32 AND MINGW AND CH_WINDOWS_STATIC_RUNTIME)
    target_link_options(community_health_tests PRIVATE
        -static-libgcc
        -static-libstdc++
    )
endif()

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
message(STATUS "Windows MinGW 静态运行时: ${CH_WINDOWS_STATIC_RUNTIME}")
message(STATUS "Windows MinGW 静态数据库客户端库: ${CH_WINDOWS_STATIC_DBCLIENT}")
message(STATUS "Windows MinGW 静态数据库客户端库: ${CH_WINDOWS_STATIC_DBCLIENT}")
