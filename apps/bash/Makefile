NAME = bash

include $(RISCV_ROOTFS_HOME)/Makefile.app

CROSS_COMPILE = riscv64-unknown-linux-gnu

BASH_REPO   = $(abspath repo)
BASH_CONFIG = $(BASH_REPO)/config.status
BASH_ELF    = $(BASH_REPO)/bash

.PHONY: init
init:
	@if [ ! -d "$(BASH_REPO)" ] || [ -z "$$(ls -A $(BASH_REPO))" ]; then \
		git submodule update --init --depth 1 $(BASH_REPO); \
    fi

$(BASH_CONFIG):
	cd $(BASH_REPO) && ./configure --host=$(CROSS_COMPILE)

$(BASH_ELF): init $(BASH_CONFIG)
	$(MAKE) -C $(BASH_REPO) >/dev/null

$(APP): $(BASH_ELF)
	ln -sf $(abspath $<) $@