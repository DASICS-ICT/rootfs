APPS = busybox hb-client hb-server
APPS_DIR = $(addprefix apps/, $(APPS))
LIBS = openssl
LIBS_DIR = $(addprefix libs/, $(LIBS))
ROOTFSIMG_DIR = $(abspath rootfsimg)
UTILS_DIR = $(abspath utils)

ifneq ($(MAKECMDGOALS),deinit)
$(shell cd $(ROOTFSIMG_DIR) && \
	mkdir -p bin dev lib proc sbin sys tmp mnt root usr usr/bin usr/sbin usr/lib var)
endif

.DEFAULT_GOAL = all

.PHONY: init deinit all $(APPS_DIR) $(LIBS_DIR) clean

init:
	git submodule update --init --depth 1

deinit:
	git submodule deinit -f --all

all: $(APPS_DIR)
	python $(UTILS_DIR)/gen_initramfs.py

$(APPS_DIR): %: $(LIBS_DIR)
	$(MAKE) -s -C $@ install

$(LIBS_DIR): %:
	$(MAKE) -s -C $@ install

clean:
	$(foreach lib, $(LIBS_DIR), $(MAKE) -s -C $(lib) clean ;)
	$(foreach app, $(APPS_DIR), $(MAKE) -s -C $(app) clean ;)
	cd $(ROOTFSIMG_DIR) && rm -f initramfs*.txt && \
		rm -rf bin dev lib proc sbin sys tmp mnt root usr var