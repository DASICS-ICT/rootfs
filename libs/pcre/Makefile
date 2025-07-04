NAME    = libpcre.a
APP     = $(DST_DIR)/lib/$(NAME)
NEED_INSTALL = false

ifneq ($(RISCV_ROOTFS_HOME),)
include $(RISCV_ROOTFS_HOME)/Makefile.app
else
$(error RISCV_ROOTFS_HOME is not defined)
endif

CROSS_COMPILE = riscv64-unknown-linux-gnu

PCRE_REPO   = $(abspath repo)
PCRE_CONFIG = $(PCRE_REPO)/config.status
PCRE_A      = $(PCRE_REPO)/.libs/libpcre.a

.PHONY: pcre

pcre: $(PCRE_CONFIG)
	$(MAKE) -C $(PCRE_REPO)

$(PCRE_CONFIG):
	cd $(PCRE_REPO) && ./configure --host=$(CROSS_COMPILE) --prefix=$(DST_DIR)

$(PCRE_A): pcre

$(APP): $(PCRE_A)
	$(MAKE) -C $(PCRE_REPO) install