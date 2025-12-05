NAME    = gdb
IST_DIR = $(abspath $(RISCV_ROOTFS_HOME)/rootfsimg/bin)
APP     = $(DST_DIR)/bin/$(NAME)

ifneq ($(RISCV_ROOTFS_HOME),)
include $(RISCV_ROOTFS_HOME)/Makefile.app
else
$(error RISCV_ROOTFS_HOME is not defined)
endif

CROSS_COMPILE = riscv64-unknown-linux-gnu

BINUTILS_REPO   = $(abspath repo)
BINUTILS_CONFIG = $(BINUTILS_REPO)/config.status

GMP_BUILD = $(abspath $(RISCV_ROOTFS_HOME)/libs/gmp/build)
MPFR_BUILD = $(abspath $(RISCV_ROOTFS_HOME)/libs/mpfr/build)

.PHONY: binutils-gdb

binutils-gdb: $(BINUTILS_CONFIG)
	$(MAKE) -C $(BINUTILS_REPO)

$(BINUTILS_CONFIG):
	cd $(BINUTILS_REPO) && \
		./configure --host=$(CROSS_COMPILE) --target=$(CROSS_COMPILE) \
			--prefix=$(DST_DIR) \
			--with-gmp=$(GMP_BUILD) --with-mpfr=$(MPFR_BUILD) \
			--with-python=no

$(APP): binutils-gdb
	$(MAKE) -C $(BINUTILS_REPO) install