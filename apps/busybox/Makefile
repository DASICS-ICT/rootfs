NAME    = busybox
IST_DIR = $(abspath $(RISCV_ROOTFS_HOME)/rootfsimg/bin)

ifneq ($(RISCV_ROOTFS_HOME),)
include $(RISCV_ROOTFS_HOME)/Makefile.app
else
$(error RISCV_ROOTFS_HOME is not defined)
endif

DEFAULT_CONFIG = config

BUSYBOX_REPO   = $(abspath repo)
BUSYBOX_CONFIG = $(BUSYBOX_REPO)/.config
BUSYBOX_ELF    = $(BUSYBOX_REPO)/busybox

.PHONY: busybox

busybox: $(BUSYBOX_CONFIG)
	$(MAKE) -C $(BUSYBOX_REPO)

$(BUSYBOX_CONFIG): $(DEFAULT_CONFIG)
	cp $(DEFAULT_CONFIG) $(BUSYBOX_CONFIG)

$(BUSYBOX_ELF): busybox

$(APP): $(BUSYBOX_ELF)
	ln -sf $(abspath $<) $@