NAME    = libz.a
APP     = $(DST_DIR)/lib/$(NAME)
NEED_INSTALL = false

ifneq ($(RISCV_ROOTFS_HOME),)
include $(RISCV_ROOTFS_HOME)/Makefile.app
else
$(error RISCV_ROOTFS_HOME is not defined)
endif

CROSS_COMPILE = riscv64-unknown-linux-gnu

ZLIB_REPO   = $(abspath repo)
ZLIB_CONFIG = $(ZLIB_REPO)/configure.log
ZLIB_A      = $(ZLIB_REPO)/libz.a

.PHONY: zlib

zlib: $(ZLIB_CONFIG)
	$(MAKE) -C $(ZLIB_REPO)

$(ZLIB_CONFIG):
	cd $(ZLIB_REPO) && CHOST=$(CROSS_COMPILE) ./configure --prefix=$(DST_DIR)

$(ZLIB_A): zlib

$(APP): $(ZLIB_A)
	$(MAKE) -C $(ZLIB_REPO) install