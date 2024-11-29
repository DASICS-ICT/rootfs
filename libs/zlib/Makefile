NAME    = libz.so.1
IST_DIR = $(abspath $(RISCV_ROOTFS_HOME)/rootfsimg/usr/lib)
APP     = $(DST_DIR)/lib/$(NAME)

ifneq ($(RISCV_ROOTFS_HOME),)
include $(RISCV_ROOTFS_HOME)/Makefile.app
else
$(error RISCV_ROOTFS_HOME is not defined)
endif

CROSS_COMPILE = riscv64-unknown-linux-gnu

ZLIB_REPO   = $(abspath repo)
ZLIB_CONFIG = $(ZLIB_REPO)/configure.log
ZLIB_SO     = $(ZLIB_REPO)/libz.so.1.3.1

.PHONY: zlib

zlib: $(ZLIB_CONFIG)
	$(MAKE) -C $(ZLIB_REPO)

$(ZLIB_CONFIG):
	cd $(ZLIB_REPO) && CHOST=$(CROSS_COMPILE) ./configure --prefix=$(DST_DIR)

$(ZLIB_SO): zlib

$(APP): $(ZLIB_SO)
	$(MAKE) -C $(ZLIB_REPO) install