NAME    = libmpfr.so.6
IST_DIR = $(abspath $(RISCV_ROOTFS_HOME)/rootfsimg/usr/lib)
APP     = $(DST_DIR)/lib/$(NAME)

ifneq ($(RISCV_ROOTFS_HOME),)
include $(RISCV_ROOTFS_HOME)/Makefile.app
else
$(error RISCV_ROOTFS_HOME is not defined)
endif

CROSS_COMPILE = riscv64-unknown-linux-gnu

MPFR_REPO   = $(abspath repo)
MPFR_CONFIG = $(MPFR_REPO)/config.status

GMP_BUILD   = $(abspath $(RISCV_ROOTFS_HOME)/libs/gmp/build)

.PHONY: libmpfr

libmpfr: $(MPFR_CONFIG)
	$(MAKE) -C $(MPFR_REPO)

$(MPFR_CONFIG):
	cd $(MPFR_REPO) && ./configure --host=$(CROSS_COMPILE) --prefix=$(DST_DIR) --with-gmp=$(GMP_BUILD)

$(APP): libmpfr
	$(MAKE) -C $(MPFR_REPO) install