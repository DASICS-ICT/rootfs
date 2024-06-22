NAME = dropbear

include $(RISCV_ROOTFS_HOME)/Makefile.app

CROSS_COMPILE = riscv64-unknown-linux-gnu

ZLIB_REPO = $(RISCV_ROOTFS_HOME)/libs/zlib/repo

DROPBEAR_REPO   = $(abspath repo)
DROPBEAR_CONFIG = $(DROPBEAR_REPO)/config.status
DROPBEAR_ELF    = $(DROPBEAR_REPO)/dropbear

override IST_DIR = $(abspath $(RISCV_ROOTFS_HOME)/rootfsimg/usr/sbin)

$(DROPBEAR_CONFIG):
	cd $(DROPBEAR_REPO) && ./configure --host=$(CROSS_COMPILE) --with-zlib=$(ZLIB_REPO)

$(DROPBEAR_ELF): $(DROPBEAR_CONFIG)
	$(MAKE) -C $(DROPBEAR_REPO)

$(APP): $(DROPBEAR_ELF)
	ln -sf $(abspath $<) $@