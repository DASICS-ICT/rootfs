NAME = busybox

include $(RISCV_ROOTFS_HOME)/Makefile.app

override IST_DIR = $(abspath $(RISCV_ROOTFS_HOME)/rootfsimg/bin)

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