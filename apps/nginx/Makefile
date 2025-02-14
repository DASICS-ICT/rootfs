NAME    = nginx
IST_DIR = $(abspath $(RISCV_ROOTFS_HOME)/rootfsimg/usr/sbin)

ifneq ($(RISCV_ROOTFS_HOME),)
include $(RISCV_ROOTFS_HOME)/Makefile.app
else
$(error RISCV_ROOTFS_HOME is not defined)
endif

export PHP=$(RISCV_ROOTFS_HOME)/apps/php/build
export PHP_CONFIG=$(PHP)/bin/php-config
export PHP_BIN=$(PHP)/bin
export PHP_INC=$(PHP)/include/php
export PHP_LIB=$(PHP)/lib

CROSS_COMPILE = riscv64-unknown-linux-gnu

NGINX_REPO   = $(abspath repo)
NGINX_CONFIG = $(NGINX_REPO)/Makefile
NGINX_ELF    = $(NGINX_REPO)/build/nginx
NGINX_FLAGS  = --prefix=$(DST_DIR) --builddir=$(NGINX_REPO)/build \
	--user=nginx --group=nginx \
	--with-cc=$(shell which $(CROSS_COMPILE)-gcc) --with-cpp=$(shell which $(CROSS_COMPILE)-g++) \
	--with-http_ssl_module \
	--with-pcre=$(RISCV_ROOTFS_HOME)/libs/pcre2/build \
	--with-openssl=$(RISCV_ROOTFS_HOME)/libs/openssl/build \
	--with-zlib=$(RISCV_ROOTFS_HOME)/libs/zlib/build \
	--with-libatomic=$(RISCV_ROOTFS_HOME)/libs/libatomic_ops/build \
	--error-log-path=/var/log/nginx/error.log \
	--http-log-path=/var/log/nginx/access.log \
	--http-client-body-temp-path=/var/lib/nginx/tmp/client_body \
	--http-fastcgi-temp-path=/var/lib/nginx/tmp/fastcgi \
	--http-proxy-temp-path=/var/lib/nginx/tmp/proxy \
	--http-scgi-temp-path=/var/lib/nginx/tmp/scgi \
	--http-uwsgi-temp-path=/var/lib/nginx/tmp/uwsgi \
	--pid-path=/var/run/nginx.pid \
	--lock-path=/var/run/lock/subsys/nginxi \
	--add-module=$(APP_DIR)/ngx-php/third_party/ngx_devel_kit \
	--add-module=$(APP_DIR)/ngx-php

.PHONY: nginx distclean

nginx: $(NGINX_CONFIG)
	$(MAKE) -C $(NGINX_REPO)

$(NGINX_CONFIG):
	cd $(NGINX_REPO) && ./auto/configure $(NGINX_FLAGS)

$(NGINX_ELF): nginx

$(APP): $(NGINX_ELF)
	ln -sf $(abspath $<) $@

clean:
	$(MAKE) -C $(NGINX_REPO) clean
