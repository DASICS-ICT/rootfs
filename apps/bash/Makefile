NAME    = bash
IST_DIR = $(abspath $(RISCV_ROOTFS_HOME)/rootfsimg/bin)

ifneq ($(RISCV_ROOTFS_HOME),)
include $(RISCV_ROOTFS_HOME)/Makefile.app
else
$(error RISCV_ROOTFS_HOME is not defined)
endif

CROSS_COMPILE = riscv64-unknown-linux-gnu

BASH_REPO   = $(abspath repo)
BASH_CONFIG = $(BASH_REPO)/config.status
BASH_ELF    = $(BASH_REPO)/bash

.PHONY: bash

bash: $(BASH_CONFIG)
	$(MAKE) -C $(BASH_REPO) >/dev/null

$(BASH_CONFIG):
	cd $(BASH_REPO) && ./configure --host=$(CROSS_COMPILE)

$(BASH_ELF): bash

$(APP): $(BASH_ELF)
	ln -sf $(abspath $<) $@