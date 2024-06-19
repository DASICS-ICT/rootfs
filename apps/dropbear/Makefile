NAME = dropbear

include $(RISCV_ROOTFS_HOME)/Makefile.app

CROSS_COMPILE = riscv64-unknown-linux-gnu

ZLIB_APP = $(RISCV_ROOTFS_HOME)/apps/zlib
ZLIB_SO  = $(ZLIB_APP)/repo/libz.so.1.3.1

DROPBEAR_REPO   = $(abspath repo)
DROPBEAR_CONFIG = $(DROPBEAR_REPO)/config.status
DROPBEAR_ELF    = $(DROPBEAR_REPO)/dropbear

.PHONY: init
init:
	@if [ ! -d "$(DROPBEAR_REPO)" ] || [ -z "$$(ls -A $(DROPBEAR_REPO))" ]; then \
		git submodule update --init --depth 1 $(DROPBEAR_REPO); \
    fi

$(ZLIB_SO):
	$(MAKE) -C $(ZLIB_APP)

$(DROPBEAR_CONFIG): $(ZLIB_SO)
	cd $(DROPBEAR_REPO) && ./configure --host=$(CROSS_COMPILE) --with-zlib=$(shell dirname $(ZLIB_SO))

$(DROPBEAR_ELF): init $(DROPBEAR_CONFIG)
	$(MAKE) -C $(DROPBEAR_REPO)

$(APP): $(DROPBEAR_ELF)
	ln -sf $(abspath $<) $@