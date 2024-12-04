include Makefile.check

APPS = busybox haveged
APPS_DIR = $(addprefix apps/, $(APPS))
LIBS = pcre
LIBS_DIR = $(addprefix libs/, $(LIBS))
ROOTFSIMG_DIR = $(abspath rootfsimg)
UTILS_DIR = $(abspath utils)

ROOTFSIMG_NEW_DIRS = bin dev lib proc sbin sys tmp mnt root \
	usr usr/bin usr/sbin usr/lib var var/run

$(shell cd $(ROOTFSIMG_DIR) && mkdir -p $(ROOTFSIMG_NEW_DIRS))

.DEFAULT_GOAL = all

.PHONY: init all $(APPS_DIR) $(LIBS_DIR) clean repoclean distclean

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

all: $(APPS_DIR)
	python $(UTILS_DIR)/gen_initramfs.py

$(APPS_DIR): %: $(LIBS_DIR)
	$(MAKE) -s -C $@ install

$(LIBS_DIR): %:
	$(MAKE) -s -C $@ install

clean:
	$(foreach dir, $(LIBS_DIR) $(APPS_DIR), $(MAKE) -s -C $(dir) clean ;)
	cd $(ROOTFSIMG_DIR) && rm -f initramfs*.txt && rm -rf $(ROOTFSIMG_NEW_DIRS)

repoclean: clean
	$(foreach dir, $(LIBS_DIR) $(APPS_DIR), \
		$(if $(wildcard $(dir)/repo/Makefile), \
			$(MAKE) -s -C $(dir)/repo clean ;) \
	)

distclean: repoclean
	git submodule deinit -f --all