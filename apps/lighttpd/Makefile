NAME    = lighttpd
IST_DIR = $(abspath $(RISCV_ROOTFS_HOME)/rootfsimg/usr/sbin)
APP     = $(DST_DIR)/sbin/lighttpd

ifneq ($(RISCV_ROOTFS_HOME),)
include $(RISCV_ROOTFS_HOME)/Makefile.app
else
$(error RISCV_ROOTFS_HOME is not defined)
endif

CROSS_COMPILE   = riscv64-unknown-linux-gnu
LIBS_DIR		= $(RISCV_ROOTFS_HOME)/libs
OPENSSL_DIR     = $(LIBS_DIR)/openssl/build
PCRE_DIR        = $(LIBS_DIR)/pcre/build

LIGHTTPD_REPO   = $(abspath repo)
LIGHTTPD_CONFIG = $(LIGHTTPD_REPO)/config.h
LIGHTTPD_ELF    = $(LIGHTTPD_REPO)/lighttpd

.PHONY: lighttpd

lighttpd: $(LIGHTTPD_CONFIG)
	$(MAKE) -C $(LIGHTTPD_REPO)

$(LIGHTTPD_CONFIG):
	cd $(LIGHTTPD_REPO) && ./autogen.sh && \
		./configure  --host=$(CROSS_COMPILE) --prefix=$(DST_DIR) --enable-shared \
			--with-pcre=$(PCRE_DIR) --with-openssl=$(OPENSSL_DIR) --without-zlib

$(LIGHTTPD_ELF): lighttpd

$(APP): $(LIGHTTPD_ELF)
	$(MAKE) -C $(LIGHTTPD_REPO) install

install:: $(APP)
	ln -sf $(DST_DIR)/lib/mod_openssl.so $(RISCV_ROOTFS_HOME)/rootfsimg/usr/lib/lighttpd/mod_openssl.so