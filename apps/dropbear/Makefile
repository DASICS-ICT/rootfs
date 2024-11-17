NAME    = dropbear
IST_DIR = $(abspath $(RISCV_ROOTFS_HOME)/rootfsimg/sbin)
APP     = $(DST_DIR)/sbin/dropbear

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
DROPBEAR_PROGS  = PROGRAMS="dropbear dbclient dropbearkey dropbearconvert scp"

.PHONY: dropbear

dropbear: $(DROPBEAR_CONFIG)
	$(MAKE) -C $(DROPBEAR_REPO) $(DROPBEAR_PROGS)

$(DROPBEAR_CONFIG):
	cd $(DROPBEAR_REPO) && ./configure --prefix=$(DST_DIR) --host=$(CROSS_COMPILE) --with-zlib=$(ZLIB_REPO)

$(DROPBEAR_ELF): dropbear
	$(MAKE) -C $(DROPBEAR_REPO) $(DROPBEAR_PROGS) install

$(APP): $(DROPBEAR_ELF)
	ln -sf $(abspath $<) $@

# Install dropbearkey & scp additionally
install:: $(APP)
	ln -sf $(abspath $(DST_DIR)/bin/dropbearkey) $(abspath $(RISCV_ROOTFS_HOME)/rootfsimg/bin/dropbearkey)
	ln -sf $(abspath $(DST_DIR)/bin/scp) $(abspath $(RISCV_ROOTFS_HOME)/rootfsimg/bin/scp)