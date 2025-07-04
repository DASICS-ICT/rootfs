NAME    = libssl.a
APP     = $(DST_DIR)/lib/$(NAME)
NEED_INSTALL = false

ifneq ($(RISCV_ROOTFS_HOME),)
include $(RISCV_ROOTFS_HOME)/Makefile.app
else
$(error RISCV_ROOTFS_HOME is not defined)
endif

OPENSSL_REPO   = $(abspath repo)
OPENSSL_CONFIG = $(OPENSSL_REPO)/configdata.pm
OPENSSL_A      = $(OPENSSL_REPO)/libssl.a

.PHONY: openssl

openssl: $(OPENSSL_CONFIG)
	$(MAKE) -C $(OPENSSL_REPO)

$(OPENSSL_CONFIG):
	cd $(OPENSSL_REPO) && ./Configure linux-generic64 no-asm no-threads no-shared\
		--cross-compile-prefix=riscv64-unknown-linux-gnu- \
		--prefix=$(DST_DIR) --openssldir=$(DST_DIR)/openssl

$(OPENSSL_A): openssl

$(APP): $(OPENSSL_A)
	# No need to install documents
	$(MAKE) -C $(OPENSSL_REPO) install_sw
