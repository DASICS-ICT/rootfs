NAME    = libcurl.so.4
IST_DIR = $(abspath $(RISCV_ROOTFS_HOME)/rootfsimg/usr/lib)
APP     = $(DST_DIR)/lib/$(NAME)

ifneq ($(RISCV_ROOTFS_HOME),)
include $(RISCV_ROOTFS_HOME)/Makefile.app
else
$(error RISCV_ROOTFS_HOME is not defined)
endif

CROSS_COMPILE = riscv64-unknown-linux-gnu

CURL_REPO   = $(abspath repo)
CURL_CONFIG = $(CURL_REPO)/curl-config
CURL_SO     = $(CURL_REPO)/lib/.libs/libcurl.so.4

.PHONY: curl

curl: $(CURL_CONFIG)
	$(MAKE) -C $(CURL_REPO)

$(CURL_CONFIG):
	cd $(CURL_REPO) && aclocal && autoconf && autoheader && \
		libtoolize --automake --copy --debug --force && \
		automake --add-missing && \
		./configure --prefix=$(DST_DIR) --host=$(CROSS_COMPILE) --without-ssl --without-libpsl --enable-debug

$(CURL_SO): curl

$(APP): $(CURL_SO)
	$(MAKE) -C $(CURL_REPO) install
