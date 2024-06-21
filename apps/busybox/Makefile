NAME = busybox

include $(RISCV_ROOTFS_HOME)/Makefile.app

DEFAULT_CONFIG = config

BUSYBOX_REPO   = $(abspath repo)
BUSYBOX_CONFIG = $(BUSYBOX_REPO)/.config
BUSYBOX_ELF    = $(BUSYBOX_REPO)/busybox

$(BUSYBOX_CONFIG): $(DEFAULT_CONFIG)
	cp $(DEFAULT_CONFIG) $(BUSYBOX_CONFIG)

$(BUSYBOX_ELF): $(BUSYBOX_CONFIG)
	$(MAKE) -C $(BUSYBOX_REPO)

$(APP): $(BUSYBOX_ELF)
	ln -sf $(abspath $<) $@