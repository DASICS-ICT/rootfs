APPS = busybox
APPS_DIR = $(addprefix apps/, $(APPS))
LIBS =
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
	@$(foreach dir,$(APPS_DIR) $(LIBS_DIR), \
		$(if $(wildcard $(dir)/repo), \
			$(if $(wildcard $(dir)/patchfile.patch), \
				echo "Applying patch to $(dir)/repo"; \
				git -C "$(dir)/repo" apply "../patchfile.patch", \
			) \
		) \
	)

deinit:
	git submodule deinit -f --all

all: $(APPS_DIR)
	python $(UTILS_DIR)/gen_initramfs.py

$(APPS_DIR): %: $(LIBS_DIR)
	$(MAKE) -s -C $@ install

$(LIBS_DIR): %:
	$(MAKE) -s -C $@ install

clean:
	$(foreach dir, $(LIBS_DIR) $(APPS_DIR), $(MAKE) -s -C $(dir) clean ;)
	cd $(ROOTFSIMG_DIR) && rm -f initramfs*.txt && \
		rm -rf bin dev lib proc sbin sys tmp mnt root usr var