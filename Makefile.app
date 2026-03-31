BUILD_TYPE ?= Release

APP_DIR ?= $(shell pwd)
INC_DIR += $(APP_DIR)/include
DST_DIR ?= $(APP_DIR)/build
IST_DIR ?= $(abspath $(RISCV_ROOTFS_HOME)/rootfsimg/root)
APP     ?= $(APP_DIR)/build/$(NAME)

NEED_INSTALL ?= true

.DEFAULT_GOAL = $(APP)

$(shell mkdir -p $(DST_DIR))

.PHONY: install clean

install:: $(APP)
ifeq ($(NEED_INSTALL),true)
	@ln -sf $< $(IST_DIR)/$(NAME)
endif

clean:
	rm -rf $(APP_DIR)/build/