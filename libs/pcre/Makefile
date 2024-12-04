NAME    = libpcre.so.1
IST_DIR = $(abspath $(RISCV_ROOTFS_HOME)/rootfsimg/usr/lib)
APP     = $(DST_DIR)/lib/$(NAME)

ifneq ($(RISCV_ROOTFS_HOME),)
include $(RISCV_ROOTFS_HOME)/Makefile.app
else
$(error RISCV_ROOTFS_HOME is not defined)
endif

CROSS_COMPILE = riscv64-unknown-linux-gnu

PCRE_REPO   = $(abspath repo)
PCRE_CONFIG = $(PCRE_REPO)/config.status
PCRE_SO     = $(PCRE_REPO)/.libs/libpcre.so.1.2.13

.PHONY: pcre

pcre: $(PCRE_CONFIG)
	$(MAKE) -C $(PCRE_REPO)

$(PCRE_CONFIG):
	cd $(PCRE_REPO) && ./configure --host=$(CROSS_COMPILE) --prefix=$(DST_DIR)

$(PCRE_SO): pcre

$(APP): $(PCRE_SO)
	$(MAKE) -C $(PCRE_REPO) install