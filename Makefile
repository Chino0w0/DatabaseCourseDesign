SHELL := /usr/bin/env bash

PROJECT_ROOT := $(CURDIR)
BUILD_DIR := $(PROJECT_ROOT)/backend/build
SERVER_BIN := $(BUILD_DIR)/community_health_server
RUN_DIR := $(PROJECT_ROOT)/.run
PID_FILE := $(RUN_DIR)/community_health_server.pid
LOG_FILE := $(RUN_DIR)/community_health_server.log
PORT ?= 8080
FRONTEND_PORT ?= 8080

DB_HOST ?= 127.0.0.1
DB_PORT ?= 3306
DB_USER ?= ch_admin
DB_PASS ?=
DB_NAME ?= community_health

export DB_HOST DB_PORT DB_USER DB_PASS DB_NAME

.PHONY: help configure build run start stop restart status logs health clean stop-port ensure-run-dir

help:
	@echo "可用目标:"
	@echo "  make configure   - 配置 CMake"
	@echo "  make build       - 编译后端服务"
	@echo "  make build-frontend - 编译前端发布包"
	@echo "  make run         - 前台运行服务（后端 + 静态托管）"
	@echo "  make start       - 后台启动服务并写入 PID / 日志"
	@echo "  make run-frontend  - 单独前台启动 Vue3 前端开发服务"
	@echo "  make start-frontend- 单独后台启动 Vue3 前端开发服务"
	@echo "  make run-frontend-old - 前台启动旧版原生前端服务 (端口 $(FRONTEND_PORT))"
	@echo "  make start-frontend-old- 后台启动旧版原生前端服务"
	@echo "  make stop-frontend - 停止后台前端服务"
	@echo "  make run-backend   - 单独前台启动后端 (等同 run)"
	@echo "  make start-backend - 单独后台启动后端 (等同 start)"
	@echo "  make stop        - 停止后台后端服务并清理 8080 端口"
	@echo "  make restart     - 重启后台后端服务"
	@echo "  make status      - 查看服务状态"
	@echo "  make logs        - 查看后台日志"
	@echo "  make health      - 检查健康接口"
	@echo "  make clean       - 删除构建目录和运行缓存"
	@echo ""
	@echo "可覆盖变量示例:"
	@echo "  make start DB_PASS='你的密码' DB_NAME=community_health"

ensure-run-dir:
	@mkdir -p "$(RUN_DIR)"

configure:
	@echo "[make] 配置 CMake"
	@cmake -S "$(PROJECT_ROOT)/backend" -B "$(BUILD_DIR)"

build: configure
	@echo "[make] 编译 community_health_server"
	@cmake --build "$(BUILD_DIR)" --target community_health_server -j 4

build-frontend:
	@echo "[make] 编译 Vue 3 前端生产包"
	@if [ ! -f "$(PROJECT_ROOT)/vue-frontend/package.json" ]; then \
		echo "[make] 未检测到 vue-frontend/package.json，跳过 Vue3 构建（运行时将自动回退 frontend_old）"; \
		exit 0; \
	fi
	@if [ ! -d "$(PROJECT_ROOT)/vue-frontend/node_modules" ]; then \
		echo "[make] 安装 NPM 依赖..."; \
		cd "$(PROJECT_ROOT)/vue-frontend" && npm install; \
	fi
	@cd "$(PROJECT_ROOT)/vue-frontend" && npm run build

stop-port:
	@if command -v fuser >/dev/null 2>&1; then \
		if fuser "$(PORT)/tcp" >/dev/null 2>&1; then \
			echo "[make] 检测到 $(PORT) 端口已占用，正在结束旧进程"; \
			fuser -k "$(PORT)/tcp" >/dev/null 2>&1 || true; \
			sleep 1; \
		fi; \
	elif command -v lsof >/dev/null 2>&1; then \
		PIDS="$$(lsof -ti tcp:$(PORT) || true)"; \
		if [[ -n "$$PIDS" ]]; then \
			echo "[make] 检测到 $(PORT) 端口已占用，正在结束旧进程"; \
			kill -9 $$PIDS >/dev/null 2>&1 || true; \
			sleep 1; \
		fi; \
	else \
		echo "[make] 未找到 fuser / lsof，跳过端口清理"; \
	fi

run: build build-frontend stop-port
	@echo "[make] 前台启动服务"
	@echo "[make] DB_HOST=$(DB_HOST) DB_PORT=$(DB_PORT) DB_USER=$(DB_USER) DB_NAME=$(DB_NAME)"
	@echo "[make] 浏览器访问: http://127.0.0.1:$(PORT)"
	@cd "$(PROJECT_ROOT)" && "$(SERVER_BIN)"

start: build build-frontend ensure-run-dir stop
	@echo "[make] 后台启动服务"
	@echo "[make] DB_HOST=$(DB_HOST) DB_PORT=$(DB_PORT) DB_USER=$(DB_USER) DB_NAME=$(DB_NAME)"
	@echo "[make] 日志文件: $(LOG_FILE)"
	@bash -lc 'cd "$(PROJECT_ROOT)" && nohup "$(SERVER_BIN)" > "$(LOG_FILE)" 2>&1 & echo $$! > "$(PID_FILE)"'
	@sleep 2
	@if [[ -f "$(PID_FILE)" ]] && kill -0 "$$(cat "$(PID_FILE)")" >/dev/null 2>&1; then \
		echo "[make] 启动成功，PID=$$(cat "$(PID_FILE)")"; \
		echo "[make] 浏览器访问: http://127.0.0.1:$(PORT)"; \
	else \
		echo "[make] 启动失败，请检查日志: $(LOG_FILE)"; \
		exit 1; \
	fi

stop: ensure-run-dir
	@if [[ -f "$(PID_FILE)" ]]; then \
		PID="$$(cat "$(PID_FILE)")"; \
		if [[ -n "$$PID" ]] && kill -0 "$$PID" >/dev/null 2>&1; then \
			echo "[make] 停止服务 PID=$$PID"; \
			kill "$$PID" >/dev/null 2>&1 || true; \
			sleep 1; \
			if kill -0 "$$PID" >/dev/null 2>&1; then \
				kill -9 "$$PID" >/dev/null 2>&1 || true; \
			fi; \
		fi; \
		rm -f "$(PID_FILE)"; \
	fi
	@$(MAKE) --no-print-directory stop-port
	@echo "[make] 服务已停止"

restart: stop start

status: ensure-run-dir
	@status_ok=1; \
	if [[ -f "$(PID_FILE)" ]]; then \
		PID="$$(cat "$(PID_FILE)")"; \
		if [[ -n "$$PID" ]] && kill -0 "$$PID" >/dev/null 2>&1; then \
			echo "[make] 服务运行中，PID=$$PID"; \
			echo "[make] 访问地址: http://127.0.0.1:$(PORT)"; \
			status_ok=0; \
		fi; \
	fi; \
	if [[ $$status_ok -ne 0 ]] && command -v fuser >/dev/null 2>&1 && fuser "$(PORT)/tcp" >/dev/null 2>&1; then \
		echo "[make] 检测到 $(PORT) 端口有进程占用，但 PID 文件缺失或已失效"; \
		status_ok=0; \
	fi; \
	if [[ $$status_ok -ne 0 ]]; then \
		echo "[make] 服务未运行"; \
	fi; \
	exit $$status_ok

logs: ensure-run-dir
	@if [[ -f "$(LOG_FILE)" ]]; then \
		tail -n 200 "$(LOG_FILE)"; \
	else \
		echo "[make] 暂无日志文件: $(LOG_FILE)"; \
	fi

health:
	@if command -v curl >/dev/null 2>&1; then \
		curl --fail --silent --show-error "http://127.0.0.1:$(PORT)/api/v1/health-check"; \
		echo; \
	else \
		echo "[make] 当前环境缺少 curl，无法执行健康检查"; \
		exit 1; \
	fi

clean: stop stop-frontend
	@echo "[make] 清理构建目录和运行缓存"
	@rm -rf "$(BUILD_DIR)" "$(RUN_DIR)"

run-frontend:
	@echo "[make] 前台启动 Vue 3 现代前端服务 (开发模式)"
	@echo "[make] 依赖: 需要系统已安装 Node.js 和 npm"
	@cd "$(PROJECT_ROOT)/vue-frontend" && npm run dev

start-frontend: ensure-run-dir
	@echo "[make] 后台启动 Vue 3 现代前端服务"
	@bash -lc 'cd "$(PROJECT_ROOT)/vue-frontend" && nohup npm run dev > "$(RUN_DIR)/frontend.log" 2>&1 & echo $$! > "$(RUN_DIR)/frontend.pid"'
	@sleep 2
	@echo "[make] 前端服务已在后台运行 (可通过 make logs 查看 npm 输出)"

stop-frontend:
	@if [[ -f "$(RUN_DIR)/frontend.pid" ]]; then \
		PID="$$(cat "$(RUN_DIR)/frontend.pid")"; \
		if kill -0 "$$PID" >/dev/null 2>&1; then \
			kill "$$PID" >/dev/null 2>&1 || true; \
			echo "[make] 前端服务 (PID=$$PID) 已停止"; \
		fi; \
		rm -f "$(RUN_DIR)/frontend.pid"; \
	fi

run-frontend-old:
	@echo "[make] 前台启动旧版结构前端服务 (端口 $(FRONTEND_PORT))"
	@echo "[make] 依赖: 需要系统已安装 python3"
	@echo "[make] 浏览器访问: http://127.0.0.1:$(FRONTEND_PORT)"
	@cd "$(PROJECT_ROOT)/frontend_old" && python3 -m http.server $(FRONTEND_PORT)

start-frontend-old: ensure-run-dir
	@echo "[make] 后台启动旧版本独立前端服务 (端口 $(FRONTEND_PORT))"
	@bash -lc 'cd "$(PROJECT_ROOT)/frontend_old" && nohup python3 -m http.server $(FRONTEND_PORT) > "$(RUN_DIR)/frontend.log" 2>&1 & echo $$! > "$(RUN_DIR)/frontend.pid"'
	@sleep 1
	@echo "[make] 浏览器访问: http://127.0.0.1:$(FRONTEND_PORT)"

run-backend: run

start-backend: start

stop-backend: stop
