#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$SCRIPT_DIR"
TARGET="${1:-run}"
shift || true

DB_HOST="${DB_HOST:-127.0.0.1}"
DB_PORT="${DB_PORT:-3306}"
DB_USER="${DB_USER:-ch_admin}"
DB_PASS="${DB_PASS:-}"
DB_NAME="${DB_NAME:-community_health}"
PORT="${PORT:-8080}"
FRONTEND_PORT="${FRONTEND_PORT:-8080}"

log() {
  printf '[start_linux] %s\n' "$1"
}

usage() {
  cat <<'EOF'
用法:
  ./start_linux.sh [target]

默认 target:
  run      前台编译并启动服务

常用 target:
  run      前台运行服务 (后端API + 静态页面托管)
  start    后台启动服务
  stop     停止服务
  restart  重启服务
  run-frontend   前台单独启动前端 (Vue3 + Vite, 端口8081)
  start-frontend 后台单独启动前端 (Vue3 + Vite)
  run-frontend-old 前台单独启动旧版原生前端 (端口8081，需python3)
  start-frontend-old 后台单独启动旧版原生前端
  run-backend    前台单独启动后端 (等同 run)
  start-backend  后台单独启动后端 (等同 start)
  status   查看服务状态
  health   调用健康检查接口
  logs     查看后台日志
  build    仅编译后端
  clean    清理构建与运行缓存
  help     查看 make 帮助

可通过环境变量覆盖数据库配置，例如:
  DB_PASS='你的密码' ./start_linux.sh run
  DB_HOST=localhost DB_PASS='你的密码' ./start_linux.sh start
EOF
}

require_cmd() {
  if ! command -v "$1" >/dev/null 2>&1; then
    printf '[start_linux] 缺少必要命令: %s\n' "$1" >&2
    exit 1
  fi
}

main() {
  require_cmd bash
  require_cmd make
  require_cmd cmake
  require_cmd git

  if [[ "$TARGET" == "-h" || "$TARGET" == "--help" || "$TARGET" == "help" ]]; then
    usage
    make -C "$PROJECT_ROOT" help
    exit 0
  fi

  log "使用 make 目标: ${TARGET}"
  log "数据库配置: host=${DB_HOST} port=${DB_PORT} user=${DB_USER} db=${DB_NAME}"
  log "后端 API 端口: ${PORT}"
  log "前端服务 端口: ${FRONTEND_PORT}"

  cd "$PROJECT_ROOT"
  DB_HOST="$DB_HOST" \
  DB_PORT="$DB_PORT" \
  DB_USER="$DB_USER" \
  DB_PASS="$DB_PASS" \
  DB_NAME="$DB_NAME" \
  PORT="$PORT" \
  FRONTEND_PORT="$FRONTEND_PORT" \
  make "$TARGET" "$@"
}

main "$@"
