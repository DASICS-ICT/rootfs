include Makefile.check

APPS = busybox haveged nginx
APPS_DIR = $(addprefix apps/, $(APPS))
LIBS = pcre2 pcre openssl zlib libatomic_ops
LIBS_DIR = $(addprefix libs/, $(LIBS))
ROOTFSIMG_DIR = $(abspath rootfsimg)
UTILS_DIR = $(abspath utils)
NETWORK ?= dhcp
NETWORK_DIR = $(abspath network)

ROOTFSIMG_NEW_DIRS = bin dev lib proc sbin sys tmp mnt root \
	usr usr/bin usr/sbin usr/lib \
	var var/run var/log var/log/nginx var/lib/nginx var/empty/nginx \
	var/lib/nginx/tmp/client_body var/lib/nginx/tmp/fastcgi \
	var/lib/nginx/tmp/proxy var/lib/nginx/tmp/scgi var/lib/nginx/tmp/uwsgi \
	var/run/lock/subsys

$(shell cd $(ROOTFSIMG_DIR) && mkdir -p $(ROOTFSIMG_NEW_DIRS))

.DEFAULT_GOAL = all

.PHONY: init all $(APPS_DIR) $(LIBS_DIR) network initramfs clean repoclean distclean

init:
	git submodule update --init --depth 1
	@$(foreach dir,$(APPS_DIR) $(LIBS_DIR), \
		$(if $(wildcard $(dir)/repo), \
			$(if $(wildcard $(dir)/patchfile.patch), \
				echo "Applying patch to $(dir)/repo"; \
				git -C "$(dir)/repo" apply "../patchfile.patch", \
			) \
		) \
	)

all: $(APPS_DIR) network
	$(MAKE) -s -C $(RISCV_ROOTFS_HOME) initramfs

$(APPS_DIR): %: $(LIBS_DIR)
	$(MAKE) -s -C $@ install

$(LIBS_DIR): %:
	$(MAKE) -s -C $@ install

network:
	$(MAKE) -s -C $(NETWORK_DIR) NETWORK=$(NETWORK)

initramfs:
	python $(UTILS_DIR)/gen_initramfs.py

clean:
	$(MAKE) -s -C $(NETWORK_DIR) clean
	$(foreach dir, $(LIBS_DIR) $(APPS_DIR), $(MAKE) -s -C $(dir) clean ;)
	cd $(ROOTFSIMG_DIR) && rm -f initramfs*.txt && rm -rf $(ROOTFSIMG_NEW_DIRS)

repoclean: clean
	$(foreach dir, $(LIBS_DIR) $(APPS_DIR), \
		$(if $(wildcard $(dir)/repo/Makefile), \
			$(MAKE) -s -C $(dir)/repo clean ;) \
	)

distclean: repoclean
	git submodule deinit -f --all
