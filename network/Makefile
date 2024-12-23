ifeq ($(RISCV_ROOTFS_HOME),)
$(error RISCV_ROOTFS_HOME is not defined)
endif

NETWORK ?= localhost
ifneq ($(NETWORK),)
ifeq ($(findstring $(NETWORK),dhcp localhost),)
$(error NETWORK must be dhcp or localhost)
endif
endif

NETWORK_DIR = $(RISCV_ROOTFS_HOME)/network/$(NETWORK)
ROOTFSIMG_ETC_DIR = $(RISCV_ROOTFS_HOME)/rootfsimg/etc
ROOTFSIMG_USR_DIR = $(RISCV_ROOTFS_HOME)/rootfsimg/usr

.DEFAULT_GOAL := all

.PHONY: all install-network clean clean-network

all: install-network

install-network:
ifneq ($(NETWORK),)
	@mkdir -p $(ROOTFSIMG_ETC_DIR)/network/if-pre-up.d
	@mkdir -p $(ROOTFSIMG_ETC_DIR)/network/if-up.d
	@mkdir -p $(ROOTFSIMG_ETC_DIR)/network/if-down.d
	@mkdir -p $(ROOTFSIMG_ETC_DIR)/network/if-post-down.d
	ln -sf $(NETWORK_DIR)/interfaces $(ROOTFSIMG_ETC_DIR)/network/interfaces
	ln -sf $(NETWORK_DIR)/setup-network.sh $(ROOTFSIMG_ETC_DIR)/init.d/setup-network.sh
	if [ -f $(NETWORK_DIR)/udhcpc.script ]; then \
		mkdir -p $(ROOTFSIMG_USR_DIR)/share/udhcpc; \
		ln -sf $(NETWORK_DIR)/udhcpc.script $(ROOTFSIMG_USR_DIR)/share/udhcpc/default.script; \
	fi
endif

clean: clean-network

clean-network:
	rm -rf $(ROOTFSIMG_ETC_DIR)/network
	rm -f $(ROOTFSIMG_ETC_DIR)/init.d/setup-network.sh
	rm -f $(ROOTFSIMG_USR_DIR)/share/udhcpc/default.script