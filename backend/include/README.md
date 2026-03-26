# 第三方头文件目录

本目录存放 C++ 单头文件第三方库，需在编译前手动下载或通过 CMake 自动获取。

## 依赖说明

| 文件 | 库名 | 版本 | 获取方式 |
|------|------|------|---------|
| `httplib.h` | [cpp-httplib](https://github.com/yhirose/cpp-httplib) | v0.15.3+ | 见下方说明 |
| `json.hpp` | [nlohmann/json](https://github.com/nlohmann/json) | v3.11.3+ | 见下方说明 |

## 手动下载方法

```bash
# 下载 cpp-httplib (单头文件)
curl -L https://raw.githubusercontent.com/yhirose/cpp-httplib/master/httplib.h \
     -o backend/include/httplib.h

# 下载 nlohmann/json (单头文件)
curl -L https://raw.githubusercontent.com/nlohmann/json/develop/single_include/nlohmann/json.hpp \
     -o backend/include/json.hpp
```

## CMake 自动下载

`CMakeLists.txt` 已通过 `FetchContent` 配置自动下载，执行 `cmake -B build` 时会自动拉取。
手动将文件放入本目录后，CMake 会优先使用本目录中的版本（通过 `target_include_directories` 配置）。

## 注意事项

- `httplib.h` 需要 OpenSSL 支持 HTTPS（本项目仅使用 HTTP，可不安装 OpenSSL）
- `json.hpp` 是纯头文件，无额外依赖
