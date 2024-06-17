NAME = busybox

include $(RISCV_ROOTFS_HOME)/Makefile.app

DEFAULT_CONFIG = config

BUSYBOX_REPO   = $(shell realpath repo)
BUSYBOX_CONFIG = $(BUSYBOX_REPO)/.config
BUSYBOX_ELF    = $(BUSYBOX_REPO)/busybox

.PHONY: init
init:
	@if [ ! -d "$(BUSYBOX_REPO)" ] || [ -z "$$(ls -A $(BUSYBOX_REPO))" ]; then \
		git submodule update --init --depth 1 $(BUSYBOX_REPO); \
    fi
	@if [ ! -f "$(BUSYBOX_CONFIG)" ] || ! diff <(tail -n +5 "$(BUSYBOX_CONFIG)") <(tail -n +5 "$(DEFAULT_CONFIG)"); then \
		cp $(DEFAULT_CONFIG) $(BUSYBOX_CONFIG); \
    fi

$(BUSYBOX_ELF): init
	$(MAKE) -C $(BUSYBOX_REPO)

$(APP): $(BUSYBOX_ELF)
	ln -sf $(abspath $<) $@