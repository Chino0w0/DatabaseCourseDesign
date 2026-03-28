#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$SCRIPT_DIR"
TARGET="run"

DB_HOST="${DB_HOST:-127.0.0.1}"
DB_PORT="${DB_PORT:-3306}"
DB_USER="${DB_USER:-ch_admin}"
DB_PASS="${DB_PASS:-}"
DB_NAME="${DB_NAME:-community_health}"
PORT="${PORT:-8080}"
FRONTEND_PORT="${FRONTEND_PORT:-8081}"
MAKE_ARGS=()

log() {
  printf '[start_linux] %s\n' "$1"
}

usage() {
  cat <<'EOF'
用法:
  ./start_linux.sh [target] [options] [-- make-args]

默认 target:
  run      前台编译并启动服务

常用 target:
  run      前台运行服务 (后端API + 静态页面托管)
  start    后台启动服务
  stop     停止服务
  restart  重启服务
  run-frontend   前台单独启动前端 (Vue3 + Vite, 默认端口8081)
  start-frontend 后台单独启动前端 (Vue3 + Vite)
  run-frontend-old 前台单独启动旧版原生前端 (默认端口8081，需python3)
  start-frontend-old 后台单独启动旧版原生前端
  run-backend    前台单独启动后端 (等同 run)
  start-backend  后台单独启动后端 (等同 start)
  status   查看服务状态
  health   调用健康检查接口
  logs     查看后台日志
  build    仅编译后端
  clean    清理构建与运行缓存
  help     查看 make 帮助

可用 options:
  --db-host=HOST
  --db-port=PORT
  --db-user=USER
  --db-pass=PASSWORD
  --db-name=NAME
  --port=PORT
  --frontend-port=PORT
  --help, -h

示例:
  ./start_linux.sh run --db-host=127.0.0.1 --db-port=3306 --db-user=ch_admin --db-pass='你的密码' --db-name=community_health --port=8080
  ./start_linux.sh start --db-pass='你的密码'
  ./start_linux.sh build -- -j 8

说明:
  1. Linux 启动入口仅负责 Linux / WSL / 服务器环境。
  2. Windows 发布包请使用 release/ 下的脚本与说明，不共用本脚本。
  3. 若 target 会启动后端服务，则必须显式提供 DB_PASS（命令行或环境变量）。
EOF
}

require_cmd() {
  if ! command -v "$1" >/dev/null 2>&1; then
    printf '[start_linux] 缺少必要命令: %s\n' "$1" >&2
    exit 1
  fi
}

has_local_backend_deps() {
  [[ -f "$PROJECT_ROOT/backend/include/httplib.h" && -f "$PROJECT_ROOT/backend/include/json.hpp" ]]
}

target_requires_db_pass() {
  case "$1" in
    run|start|restart|run-backend|start-backend)
      return 0
      ;;
    *)
      return 1
      ;;
  esac
}

parse_args() {
  while [[ $# -gt 0 ]]; do
    case "$1" in
      -h|--help)
        TARGET="help"
        shift
        ;;
      --db-host=*)
        DB_HOST="${1#*=}"
        shift
        ;;
      --db-port=*)
        DB_PORT="${1#*=}"
        shift
        ;;
      --db-user=*)
        DB_USER="${1#*=}"
        shift
        ;;
      --db-pass=*)
        DB_PASS="${1#*=}"
        shift
        ;;
      --db-name=*)
        DB_NAME="${1#*=}"
        shift
        ;;
      --port=*)
        PORT="${1#*=}"
        shift
        ;;
      --frontend-port=*)
        FRONTEND_PORT="${1#*=}"
        shift
        ;;
      --)
        shift
        while [[ $# -gt 0 ]]; do
          MAKE_ARGS+=("$1")
          shift
        done
        ;;
      -* )
        printf '[start_linux] 不支持的参数: %s\n' "$1" >&2
        usage
        exit 1
        ;;
      *)
        if [[ "$TARGET" == "run" ]]; then
          TARGET="$1"
        else
          MAKE_ARGS+=("$1")
        fi
        shift
        ;;
    esac
  done
}

main() {
  parse_args "$@"

  require_cmd bash
  require_cmd make
  require_cmd cmake

  if has_local_backend_deps; then
    log "检测到本地第三方头文件，CMake 将优先使用 backend/include 中的依赖"
  elif command -v git >/dev/null 2>&1; then
    log "未检测到完整本地第三方头文件，将允许 CMake 通过 FetchContent 拉取依赖"
  else
    printf '[start_linux] 未检测到完整本地第三方头文件，且当前环境缺少 git，无法回退下载依赖。\n' >&2
    printf '[start_linux] 请补齐 backend/include/httplib.h 与 backend/include/json.hpp，或安装 git 后重试。\n' >&2
    exit 1
  fi

  if [[ "$TARGET" == "help" ]]; then
    usage
    make -C "$PROJECT_ROOT" help
    exit 0
  fi

  if target_requires_db_pass "$TARGET" && [[ -z "$DB_PASS" ]]; then
    printf '[start_linux] target=%s 需要数据库密码，请通过 --db-pass=... 或环境变量 DB_PASS 提供。\n' "$TARGET" >&2
    exit 1
  fi

  log "使用 make 目标: ${TARGET}"
  log "数据库配置: host=${DB_HOST} port=${DB_PORT} user=${DB_USER} db=${DB_NAME}"
  log "后端 API 端口: ${PORT}"
  log "前端服务端口: ${FRONTEND_PORT}"

  cd "$PROJECT_ROOT"
  DB_HOST="$DB_HOST" \
  DB_PORT="$DB_PORT" \
  DB_USER="$DB_USER" \
  DB_PASS="$DB_PASS" \
  DB_NAME="$DB_NAME" \
  PORT="$PORT" \
  FRONTEND_PORT="$FRONTEND_PORT" \
  make "$TARGET" "${MAKE_ARGS[@]}"
}

main "$@"
