NAME    = busybox
IST_DIR = $(abspath $(RISCV_ROOTFS_HOME)/rootfsimg/bin)

ifneq ($(RISCV_ROOTFS_HOME),)
include $(RISCV_ROOTFS_HOME)/Makefile.app
else
$(error RISCV_ROOTFS_HOME is not defined)
endif

ifeq ($(BUILD_TYPE),Debug)
DEFAULT_CONFIG = config_debug
else
DEFAULT_CONFIG = config
endif

BUSYBOX_REPO   = $(abspath repo)
BUSYBOX_CONFIG = $(BUSYBOX_REPO)/.config
BUSYBOX_ELF    = $(BUSYBOX_REPO)/busybox

.PHONY: busybox

BUSYBOX_MAKE_FLAGS =
ifeq ($(BUILD_TYPE),Debug)
BUSYBOX_MAKE_FLAGS += SKIP_STRIP=y
endif

busybox: $(BUSYBOX_CONFIG)
	$(MAKE) -C $(BUSYBOX_REPO) $(BUSYBOX_MAKE_FLAGS)

$(BUSYBOX_CONFIG): $(DEFAULT_CONFIG)
	cp $(DEFAULT_CONFIG) $(BUSYBOX_CONFIG)

$(BUSYBOX_ELF): busybox

$(APP): busybox
	ln -sf $(abspath $(BUSYBOX_ELF)) $@