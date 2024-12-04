NAME    = libssl.so.1.0.0
IST_DIR = $(abspath $(RISCV_ROOTFS_HOME)/rootfsimg/usr/lib)
APP     = $(DST_DIR)/lib/$(NAME)

ifneq ($(RISCV_ROOTFS_HOME),)
include $(RISCV_ROOTFS_HOME)/Makefile.app
else
$(error RISCV_ROOTFS_HOME is not defined)
endif

OPENSSL_REPO   = $(abspath repo)
OPENSSL_CONFIG = $(OPENSSL_REPO)/Makefile
OPENSSL_SO     = $(OPENSSL_REPO)/libssl.so.1.0.0

.PHONY: openssl

openssl: $(OPENSSL_CONFIG)
	# NOTE: DO NOT use multiple-threading to 'make' due to issue https://github.com/bilibili/ijkplayer/issues/5113
	$(MAKE) -C $(OPENSSL_REPO) -j1

$(OPENSSL_CONFIG):
	cd $(OPENSSL_REPO) && ./Configure linux-generic64 no-asm no-threads shared\
		--cross-compile-prefix=riscv64-unknown-linux-gnu- \
		--prefix=$(DST_DIR) --openssldir=$(DST_DIR)/openssl

$(OPENSSL_SO): openssl

$(APP): $(OPENSSL_SO)
	# NOTE: 'make install' will fail due to issue https://github.com/openssl/openssl/issues/57
	$(MAKE) -C $(OPENSSL_REPO) install_sw

# libssl.so.1.0.0 is installed in Makefile.app, thus here we install libcrypto.so.1.0.0
install:: $(APP)
	@ln -sf $(DST_DIR)/lib/libcrypto.so.1.0.0 $(IST_DIR)/libcrypto.so.1.0.0