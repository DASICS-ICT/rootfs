NAME    = libgmp.so.10
IST_DIR = $(abspath $(RISCV_ROOTFS_HOME)/rootfsimg/usr/lib)
APP     = $(DST_DIR)/lib/$(NAME)

ifneq ($(RISCV_ROOTFS_HOME),)
include $(RISCV_ROOTFS_HOME)/Makefile.app
else
$(error RISCV_ROOTFS_HOME is not defined)
endif

CROSS_COMPILE = riscv64-unknown-linux-gnu

GMP_REPO = $(abspath repo)
GMP_CONFIG = $(GMP_REPO)/config.status

.PHONY: libgmp

libgmp: $(GMP_CONFIG)
	$(MAKE) -C $(GMP_REPO)

$(GMP_CONFIG):
	cd $(GMP_REPO) && ./configure --host=$(CROSS_COMPILE) --prefix=$(DST_DIR)

$(APP): libgmp
	$(MAKE) -C $(GMP_REPO) install