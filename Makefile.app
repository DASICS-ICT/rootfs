APP_DIR ?= $(shell pwd)
INC_DIR += $(APP_DIR)/include
DST_DIR ?= $(APP_DIR)/build
IST_DIR ?= $(abspath $(RISCV_ROOTFS_HOME)/rootfsimg/root)
APP     ?= $(APP_DIR)/build/$(NAME)

.DEFAULT_GOAL = $(APP)

$(shell mkdir -p $(DST_DIR))

.PHONY: install clean

install:: $(APP)
	@ln -sf $< $(IST_DIR)/$(NAME)

clean:
	rm -rf $(APP_DIR)/build/