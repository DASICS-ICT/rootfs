NAME = libz.so.1

include $(RISCV_ROOTFS_HOME)/Makefile.app

CROSS_COMPILE = riscv64-unknown-linux-gnu

ZLIB_REPO   = $(abspath repo)
ZLIB_CONFIG = $(ZLIB_REPO)/configure.log
ZLIB_SO     = $(ZLIB_REPO)/libz.so.1.3.1

override IST_DIR = $(abspath $(RISCV_ROOTFS_HOME)/rootfsimg/usr/lib)

.PHONY: zlib

zlib: $(ZLIB_CONFIG)
	$(MAKE) -C $(ZLIB_REPO)

$(ZLIB_CONFIG):
	cd $(ZLIB_REPO) && CHOST=$(CROSS_COMPILE) ./configure

$(ZLIB_SO): zlib

$(APP): $(ZLIB_SO)
	ln -sf $(abspath $<) $@