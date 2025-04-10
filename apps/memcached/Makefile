NAME    = memcached
IST_DIR = $(abspath $(RISCV_ROOTFS_HOME)/rootfsimg/usr/bin)
APP     = $(DST_DIR)/bin/$(NAME)

ifneq ($(RISCV_ROOTFS_HOME),)
include $(RISCV_ROOTFS_HOME)/Makefile.app
else
$(error RISCV_ROOTFS_HOME is not defined)
endif

CROSS_COMPILE = riscv64-unknown-linux-gnu

MEMCACHED_REPO   = $(abspath repo)
MEMCACHED_CONFIG = $(MEMCACHED_REPO)/config.status
MEMCACHED_ELF    = $(MEMCACHED_REPO)/memcached

LIBEVENT_BUILD   = $(abspath $(RISCV_ROOTFS_HOME)/libs/libevent/build)

.PHONY: memcached

memcached: $(MEMCACHED_CONFIG)
	$(MAKE) -C $(MEMCACHED_REPO)

$(MEMCACHED_CONFIG):
	cd $(MEMCACHED_REPO) && ./autogen.sh && \
		./configure --host=$(CROSS_COMPILE) --prefix=$(DST_DIR) --with-libevent=$(LIBEVENT_BUILD)

$(MEMCACHED_ELF): memcached
	# FIXME: Deleting this comment will cause some fucking compiler errors.

$(APP): $(MEMCACHED_ELF)
	$(MAKE) -C $(MEMCACHED_REPO) install