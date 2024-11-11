NAME    = dropbear
IST_DIR = $(abspath $(RISCV_ROOTFS_HOME)/rootfsimg/usr/sbin)

ifneq ($(RISCV_ROOTFS_HOME),)
include $(RISCV_ROOTFS_HOME)/Makefile.app
else
$(error RISCV_ROOTFS_HOME is not defined)
endif

CROSS_COMPILE = riscv64-unknown-linux-gnu

ZLIB_REPO = $(RISCV_ROOTFS_HOME)/libs/zlib/repo

DROPBEAR_REPO   = $(abspath repo)
DROPBEAR_CONFIG = $(DROPBEAR_REPO)/config.status
DROPBEAR_ELF    = $(DROPBEAR_REPO)/dropbear

.PHONY: dropbear

dropbear: $(DROPBEAR_CONFIG)
	$(MAKE) -C $(DROPBEAR_REPO)

$(DROPBEAR_CONFIG):
	cd $(DROPBEAR_REPO) && ./configure --host=$(CROSS_COMPILE) --with-zlib=$(ZLIB_REPO)

$(DROPBEAR_ELF): dropbear

$(APP): $(DROPBEAR_ELF)
	ln -sf $(abspath $<) $@