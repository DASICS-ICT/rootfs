NAME    = haveged
IST_DIR = $(abspath $(RISCV_ROOTFS_HOME)/rootfsimg/usr/sbin)
APP     = $(DST_DIR)/sbin/haveged

ifneq ($(RISCV_ROOTFS_HOME),)
include $(RISCV_ROOTFS_HOME)/Makefile.app
else
$(error RISCV_ROOTFS_HOME is not defined)
endif

CROSS_COMPILE = riscv64-unknown-linux-gnu

HAVEGED_REPO   = $(abspath repo)
HAVEGED_CONFIG = $(HAVEGED_REPO)/config.h
HAVEGED_ELF    = $(HAVEGED_REPO)/.lib/haveged

.PHONY: haveged

haveged: $(HAVEGED_CONFIG)
	$(MAKE) -C $(HAVEGED_REPO)

$(HAVEGED_CONFIG):
	cd $(HAVEGED_REPO) && autoreconf && ./configure --host=$(CROSS_COMPILE) --prefix=$(DST_DIR)

$(HAVEGED_ELF): haveged

$(APP): $(HAVEGED_ELF)
	$(MAKE) -C $(HAVEGED_REPO) install

install:: $(APP)
	ln -sf $(DST_DIR)/lib/libhavege.so.2 $(abspath $(RISCV_ROOTFS_HOME)/rootfsimg/usr/lib/libhavege.so.2)