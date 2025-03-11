NAME = libDASICS.a

ifeq ($(RISCV_ROOTFS_HOME),)
$(error RISCV_ROOTFS_HOME is not defined)
endif
APP_DIR ?= $(shell pwd)
APP     ?= $(APP_DIR)/build/$(NAME)
DST_DIR ?= $(APP_DIR)/build
$(shell mkdir -p $(DST_DIR))

LIBDASICS_REPO = $(abspath repo)
LIBDASICS_A    = $(LIBDASICS_REPO)/build/libDASICS.a

.PHONY: libDASICS install clean

libDASICS:
	$(MAKE) -C $(LIBDASICS_REPO)

$(LIBDASICS_A): libDASICS

$(APP): $(LIBDASICS_A)
	ln -sf $(abspath $<) $@
	ln -sf $(LIBDASICS_REPO)/include $(DST_DIR)/include
	ln -sf $(LIBDASICS_REPO)/ld.lds $(DST_DIR)/ld.lds

install: $(APP)
	# do noting

clean:
	rm -rf $(APP_DIR)/build/