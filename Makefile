APPS = busybox
APPS_DIR = $(addprefix apps/, $(APPS))
ROOTFSIMG_DIR = $(abspath rootfsimg)
UTILS_DIR = $(abspath utils)

$(shell cd $(ROOTFSIMG_DIR) && \
		mkdir -p bin dev lib proc sbin sys tmp mnt root usr usr/bin usr/sbin var)

.DEFAULT_GOAL = all

.PHONY: init all $(APPS_DIR) clean

init:
	git submodule update --init --depth 1

all: $(APPS_DIR)
	python $(UTILS_DIR)/gen_initramfs.py

$(APPS_DIR): %:
	$(MAKE) -s -C $@ install

clean:
	$(foreach app, $(APPS_DIR), $(MAKE) -s -C $(app) clean ;)
	cd $(ROOTFSIMG_DIR) && rm -f initramfs*.txt && \
		rm -rf bin dev lib proc sbin sys tmp mnt root usr var