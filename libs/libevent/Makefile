NAME    = libevent-2.1.so.7
IST_DIR = $(abspath $(RISCV_ROOTFS_HOME)/rootfsimg/usr/lib)
APP     = $(DST_DIR)/lib/$(NAME)

ifneq ($(RISCV_ROOTFS_HOME),)
include $(RISCV_ROOTFS_HOME)/Makefile.app
else
$(error RISCV_ROOTFS_HOME is not defined)
endif

CROSS_COMPILE = riscv64-unknown-linux-gnu

LIBEVENT_REPO   = $(abspath repo)
LIBEVENT_CONFIG = $(LIBEVENT_REPO)/config.status
LIBEVENT_SO     = $(LIBEVENT_REPO)/.libs/libevent-2.1.so.7

OPENSSL_PKGCONFIG = $(abspath $(RISCV_ROOTFS_HOME)/libs/openssl/build/lib/pkgconfig)

.PHONY: libevent

libevent: $(LIBEVENT_CONFIG)
	$(MAKE) -C $(LIBEVENT_REPO)

$(LIBEVENT_CONFIG):
	cd $(LIBEVENT_REPO) && ./autogen.sh && \
		./configure --host=$(CROSS_COMPILE) --prefix=$(DST_DIR) PKG_CONFIG_PATH=$(OPENSSL_PKGCONFIG)

$(LIBEVENT_SO): libevent

$(APP): $(LIBEVENT_SO)
	$(MAKE) -C $(LIBEVENT_REPO) install