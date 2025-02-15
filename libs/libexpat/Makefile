NAME = libexpat.a
APP  = $(DST_DIR)/lib/$(NAME)
NEED_INSTALL = false

ifneq ($(RISCV_ROOTFS_HOME),)
include $(RISCV_ROOTFS_HOME)/Makefile.app
else
$(error RISCV_ROOTFS_HOME is not defined)
endif

CROSS_COMPILE = riscv64-unknown-linux-gnu-

LIBEXPAT_REPO   = $(abspath repo)/expat
LIBEXPAT_CONFIG = $(LIBEXPAT_REPO)/config.status
LIBEXPAT_A      = $(LIBEXPAT_REPO)/.libs/libexpat.a

.PHONY: libexpat

libexpat: $(LIBEXPAT_CONFIG)
	$(MAKE) -C $(LIBEXPAT_REPO)

$(LIBEXPAT_CONFIG):
	cd $(LIBEXPAT_REPO) && ./buildconf.sh && \
		./configure --prefix=$(DST_DIR) --host=riscv64 CC=$(CROSS_COMPILE)gcc

$(LIBEXPAT_A): libexpat

$(APP): $(LIBEXPAT_A)
	$(MAKE) -C $(LIBEXPAT_REPO) install