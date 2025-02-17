NAME = php
IST_DIR = $(abspath $(RISCV_ROOTFS_HOME)/rootfsimg/usr/bin)
APP = $(DST_DIR)/bin/php

LIB = $(DST_DIR)/lib/libphp7.so

ifneq ($(RISCV_ROOTFS_HOME),)
include $(RISCV_ROOTFS_HOME)/Makefile.app
else
$(error RISCV_ROOTFS_HOME is not defined)
endif

CROSS_COMPILE = riscv64-unknown-linux-gnu
PCRE_DST = $(RISCV_ROOTFS_HOME)/libs/pcre/build

PHP_REPO = $(abspath repo)
PHP_CONFIG = $(PHP_REPO)/Makefile
PHP_FLAGS = --prefix=$(DST_DIR) --host=$(CROSS_COMPILE) --enable-embed \
	--without-pcre-jit --with-pcre-regex=$(PCRE_DST) \
	--disable-all --without-valgrind


$(PHP_CONFIG):
	cd $(PHP_REPO) && ./buildconf --force && ./configure $(PHP_FLAGS)

$(APP): $(PHP_CONFIG)
	$(MAKE) -C $(PHP_REPO) install

install:: $(APP)
	ln -sf $(DST_DIR)/lib/libphp7.so $(abspath $(RISCV_ROOTFS_HOME)/rootfsimg/usr/lib/libphp7.so)


.PHONY: distclean

distclean:
	rm -rf $(APP_DIR)/build && $(MAKE) -C $(PHP_REPO) clean
