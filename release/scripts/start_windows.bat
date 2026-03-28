@echo off
setlocal EnableExtensions DisableDelayedExpansion

REM ============================================================
REM 社区健康档案管理系统 - Windows 发布包启动脚本
REM 说明：
REM 1. 请先将 ..\config\app.env.example 复制为 ..\config\app.env
REM 2. 再按实际 MySQL 信息修改 app.env
REM 3. 本脚本会读取配置并启动后端服务
REM ============================================================

set "SCRIPT_DIR=%~dp0"
for %%I in ("%SCRIPT_DIR%..") do set "RELEASE_ROOT=%%~fI"
set "CONFIG_FILE=%RELEASE_ROOT%\config\app.env"
set "CONFIG_TEMPLATE=%RELEASE_ROOT%\config\app.env.example"
set "APP_DIR=%RELEASE_ROOT%\app"
set "SERVER_EXE=%APP_DIR%\community_health_server.exe"

echo [release] 发布目录: %RELEASE_ROOT%

if not exist "%SERVER_EXE%" (
  echo [release] 错误：未找到后端程序 %SERVER_EXE%
  echo [release] 请先将编译好的 community_health_server.exe 放入 release\app 目录
  exit /b 1
)

if not exist "%CONFIG_FILE%" (
  echo [release] 错误：未找到配置文件 %CONFIG_FILE%
  if exist "%CONFIG_TEMPLATE%" (
    echo [release] 请先复制 app.env.example 为 app.env，再修改数据库连接信息
    echo [release] 示例命令：copy "%CONFIG_TEMPLATE%" "%CONFIG_FILE%"
  )
  exit /b 1
)

echo [release] 正在读取配置文件...
for /f "usebackq tokens=1,* delims== eol=#" %%A in ("%CONFIG_FILE%") do (
  call :set_config_var "%%A" "%%B"
)

if not defined DB_HOST set "DB_HOST=127.0.0.1"
if not defined DB_PORT set "DB_PORT=3306"
if not defined DB_USER set "DB_USER=ch_admin"
if not defined DB_NAME set "DB_NAME=community_health"
if not defined PORT set "PORT=8080"

if not defined DB_PASS (
  echo [release] 错误：配置文件中未设置 DB_PASS，请在 app.env 中填写真实数据库密码
  exit /b 1
)

echo [release] 当前配置：
echo [release] DB_HOST=%DB_HOST%
echo [release] DB_PORT=%DB_PORT%
echo [release] DB_USER=%DB_USER%
echo [release] DB_NAME=%DB_NAME%
echo [release] PORT=%PORT%
echo.
echo [release] 请确认 MySQL 已启动，且数据库脚本已导入完成。
echo [release] 启动后可在浏览器访问 http://127.0.0.1:%PORT%
echo.

pushd "%APP_DIR%"
"%SERVER_EXE%"
set "EXIT_CODE=%ERRORLEVEL%"
popd

echo.
echo [release] 服务已退出，退出码：%EXIT_CODE%
exit /b %EXIT_CODE%

:set_config_var
set "RAW_KEY=%~1"
set "RAW_VALUE=%~2"

if "%RAW_KEY%"=="" goto :eof

for /f "tokens=* delims= " %%K in ("%RAW_KEY%") do set "RAW_KEY=%%K"
if "%RAW_KEY%"=="" goto :eof

set "%RAW_KEY%=%RAW_VALUE%"
goto :eof
