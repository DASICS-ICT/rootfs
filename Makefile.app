ifeq ($(RISCV_ROOTFS_HOME),)
	$(error RISCV_ROOTFS_HOME is not defined)
endif

APP_DIR ?= $(shell pwd)
INC_DIR += $(APP_DIR)/include/
DST_DIR ?= $(APP_DIR)/build/
APP     ?= $(APP_DIR)/build/$(NAME)

.DEFAULT_GOAL = $(APP)

$(shell mkdir -p $(DST_DIR))

.PHONY: install clean

install: $(APP)
	@ln -sf $(APP) $(RISCV_ROOTFS_HOME)/rootfsimg/build/$(NAME)

clean:
	rm -rf $(APP_DIR)/build/