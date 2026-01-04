include Makefile.check

APPS = busybox dpdk
APPS_DIR = $(addprefix apps/, $(APPS))
LIBS = openssl libfuse libuuid 
LIBS_DIR = $(addprefix libs/, $(LIBS))
LIBS_DEP =
LIBS_DEP_DIR = $(addprefix libs/, $(LIBS_DEP))
ROOTFSIMG_DIR = $(abspath rootfsimg)
UTILS_DIR = $(abspath utils)
NETWORK ?=
NETWORK_DIR = $(abspath network)

ROOTFSIMG_NEW_DIRS = bin dev dev/pts lib proc sbin sys tmp mnt root \
	usr usr/bin usr/sbin usr/lib var var/run

$(shell cd $(ROOTFSIMG_DIR) && mkdir -p $(ROOTFSIMG_NEW_DIRS))

.DEFAULT_GOAL = all

.PHONY: init all $(APPS_DIR) $(APPS_DEP_DIR) $(LIBS_DIR) $(LIBS_DEP_DIR) network initramfs clean repoclean distclean

init:
	git submodule update --init --depth 1
	@$(foreach dir,$(APPS_DIR) $(APPS_DEP_DIR) $(LIBS_DIR) $(LIBS_DEP_DIR), \
		$(if $(wildcard $(dir)/repo), \
			$(if $(wildcard $(dir)/patchfile.patch), \
				echo "Applying patch to $(dir)/repo"; \
				git -C "$(dir)/repo" apply "../patchfile.patch"; \
			) \
		) \
	)

all: $(APPS_DIR) network
	$(MAKE) -s -C $(RISCV_ROOTFS_HOME) initramfs

$(APPS_DIR): %: $(LIBS_DIR) $(LIBS_DEP_DIR)
	$(MAKE) -s -C $@ install

$(LIBS_DIR): %:
	$(MAKE) -s -C $@ install

$(APPS_DEP_DIR): %: $(APPS_DIR)
	$(MAKE) -s -C $@ install

$(LIBS_DEP_DIR): %: $(LIBS_DIR)
	$(MAKE) -s -C $@ install

network:
	$(MAKE) -s -C $(NETWORK_DIR) NETWORK=$(NETWORK)

initramfs:
	python $(UTILS_DIR)/gen_initramfs.py

clean:
	$(MAKE) -s -C $(NETWORK_DIR) clean
	$(foreach dir, $(LIBS_DIR) $(LIBS_DEP_DIR) $(APPS_DIR) $(APPS_DEP_DIR), $(MAKE) -s -C $(dir) clean ;)
	cd $(ROOTFSIMG_DIR) && rm -f initramfs*.txt && rm -rf $(ROOTFSIMG_NEW_DIRS)

repoclean: clean
	$(foreach dir, $(LIBS_DIR) $(LIBS_DEP_DIR) $(APPS_DIR) $(APPS_DEP_DIR), \
		$(if $(wildcard $(dir)/repo/Makefile), \
			$(MAKE) -s -C $(dir)/repo clean ;) \
	)

distclean: repoclean
	git submodule deinit -f --all