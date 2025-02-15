NAME    = libaprutil-1.a
IST_DIR = $(abspath $(RISCV_ROOTFS_HOME)/rootfsimg/usr/lib)
APP     = $(DST_DIR)/lib/$(NAME)

ifneq ($(RISCV_ROOTFS_HOME),)
include $(RISCV_ROOTFS_HOME)/Makefile.app
else
$(error RISCV_ROOTFS_HOME is not defined)
endif

CROSS_COMPILE = riscv64-unknown-linux-gnu-

APRUTIL_REPO   = $(abspath repo)
APRUTIL_CONFIG = $(APRUTIL_REPO)/config.status
APRUTIL_A      = $(APRUTIL_REPO)/.libs/libaprutil-1.a

APR_DIR      = $(abspath $(RISCV_ROOTFS_HOME)/libs/apr)
LIBEXPAT_DIR = $(abspath $(RISCV_ROOTFS_HOME)/libs/libexpat)

.PHONY: aprutil

aprutil: $(APRUTIL_CONFIG)
	$(MAKE) -C $(APRUTIL_REPO)

$(APRUTIL_CONFIG):
	cd $(APRUTIL_REPO) && ./buildconf --with-apr=$(APR_DIR)/repo && \
		./configure --prefix=$(DST_DIR) --host=riscv64 CC=$(CROSS_COMPILE)gcc \
			--with-apr=$(APR_DIR)/build --with-expat=$(LIBEXPAT_DIR)/build

$(APRUTIL_A): aprutil

$(APP): $(APRUTIL_A)
	$(MAKE) -C $(APRUTIL_REPO) install