NAME    = libssl.so.3
IST_DIR = $(abspath $(RISCV_ROOTFS_HOME)/rootfsimg/usr/lib)
APP     = $(DST_DIR)/lib/$(NAME)

ifneq ($(RISCV_ROOTFS_HOME),)
include $(RISCV_ROOTFS_HOME)/Makefile.app
else
$(error RISCV_ROOTFS_HOME is not defined)
endif

OPENSSL_REPO   = $(abspath repo)
OPENSSL_CONFIG = $(OPENSSL_REPO)/configdata.pm
OPENSSL_SO     = $(OPENSSL_REPO)/libssl.so.3

.PHONY: openssl

openssl: $(OPENSSL_CONFIG)
	$(MAKE) -C $(OPENSSL_REPO)

$(OPENSSL_CONFIG):
	cd $(OPENSSL_REPO) && ./Configure linux-generic64 no-asm no-threads shared\
		--cross-compile-prefix=riscv64-unknown-linux-gnu- \
		--prefix=$(DST_DIR) --openssldir=$(DST_DIR)/openssl

$(OPENSSL_SO): openssl

$(APP): $(OPENSSL_SO)
	# No need to install documents
	$(MAKE) -C $(OPENSSL_REPO) install_sw

# libssl.so.3 is installed in Makefile.app, thus here we install libcrypto.so.3
install:: $(APP)
	@ln -sf $(DST_DIR)/lib/libcrypto.so.3 $(IST_DIR)/libcrypto.so.3