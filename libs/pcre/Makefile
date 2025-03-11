URL         := https://github.com/DASICS-ICT/pcre/releases/download/pcre-8.37-release/pcre-8.37.tar.bz2
TAR_FILE    := $(notdir $(URL))
APP_DIR 	:= $(shell pwd)
REPO_DIR    := repo
DST_DIR		:= $(APP_DIR)/build
PATCH_FILE  := fix.patch  # 补丁文件名
CROSS_COMPILE = riscv64-unknown-linux-gnu

.PHONY: clean build install

all: install

$(TAR_FILE):
	@echo "正在下载压缩包..."
	@curl -OL $(URL) || wget -q $(URL)

$(REPO_DIR)/.extracted: $(TAR_FILE)
	@mkdir -p $(REPO_DIR)
	@tar xvf $< -C $(REPO_DIR) --strip-components=1
	@touch $@

# 补丁应用标记文件
$(REPO_DIR)/.patched: $(REPO_DIR)/.extracted $(PATCH_FILE)
	@echo "应用补丁 $(PATCH_FILE)..."
	@cd $(REPO_DIR) && patch -p1 < ../$(PATCH_FILE)
	@touch $@

$(REPO_DIR)/.configured: $(REPO_DIR)/.patched
	@cd $(REPO_DIR) && ./configure --host=$(CROSS_COMPILE) --prefix=$(DST_DIR) --enable-unicode-properties CFLAGS="-g" CXXFLAGS="-g"
	@touch $@

build: $(REPO_DIR)/.configured
	@echo "开始编译..."
	@$(MAKE) -C $(REPO_DIR)
	@$(MAKE) -C $(REPO_DIR) install

clean:
	@rm -rf $(REPO_DIR)
	@rm -rf $(DST_DIR)
	@echo "已清理构建文件"

NAME = libpcre.so.1
TARGET_FILE = $(DST_DIR)/lib/$(NAME)
ifeq ($(RISCV_ROOTFS_HOME),)
$(error RISCV_ROOTFS_HOME is not defined)
endif
IST_DIR = $(abspath $(RISCV_ROOTFS_HOME)/rootfsimg/usr/lib)

$(TARGET_FILE): build

install:: $(TARGET_FILE)
	@ln -sf $< $(IST_DIR)/$(NAME)
	@echo "已安装至 $(IST_DIR)/$(NAME)"

