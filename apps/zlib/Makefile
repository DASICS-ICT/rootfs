NAME = libz.so.1.3.1

include $(RISCV_ROOTFS_HOME)/Makefile.app

CROSS_COMPILE = riscv64-unknown-linux-gnu

ZLIB_REPO   = $(abspath repo)
ZLIB_CONFIG = $(ZLIB_REPO)/configure.log
ZLIB_SO     = $(ZLIB_REPO)/libz.so.1.3.1

.PHONY: init
init:
	@if [ ! -d "$(ZLIB_REPO)" ] || [ -z "$$(ls -A $(ZLIB_REPO))" ]; then \
		git submodule update --init --depth 1 $(ZLIB_REPO); \
    fi

$(ZLIB_CONFIG):
	cd $(ZLIB_REPO) && CHOST=$(CROSS_COMPILE) ./configure

$(ZLIB_SO): init $(ZLIB_CONFIG)
	$(MAKE) -C $(ZLIB_REPO)

$(APP): $(ZLIB_SO)
	ln -sf $(abspath $<) $@