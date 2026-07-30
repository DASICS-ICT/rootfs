include Makefile.check

PROFILE ?= base

BASE_APPS = busybox
RUNTIME_REGRESSION_APPS = dasics-test/utmod dasics-test/tmod \
	dasics-test/d3-utmod dasics-test/d3-tmod \
	dasics-test/d4-utmod dasics-test/d4-tmod \
	dasics-test/e1-utmod dasics-test/e1-tmod \
	dasics-test/e5-utmod dasics-test/e5-tmod \
	dasics-test/f1-utmod dasics-test/f1-tmod
DUMMY_APPS = dasics-test/dummy-glue dasics-test/dummy-logic
LEGACY_APPS = dasics-test/rwx dasics-test/free dasics-test/ofb \
	dasics-test/vnic

ifeq ($(PROFILE),base)
PROFILE_APPS =
else ifeq ($(PROFILE),runtime-regression)
PROFILE_APPS = $(RUNTIME_REGRESSION_APPS)
else ifeq ($(PROFILE),dummy)
PROFILE_APPS = $(DUMMY_APPS)
else ifeq ($(PROFILE),legacy)
PROFILE_APPS = $(LEGACY_APPS)
else
$(error Unknown PROFILE '$(PROFILE)'; use base, runtime-regression, dummy, or legacy)
endif

APPS = $(BASE_APPS) $(PROFILE_APPS)
APPS_DIR = $(addprefix apps/, $(APPS))
LIBS = LibDASICS
LIBS_DIR = $(addprefix libs/, $(LIBS))
LIBS_DEP =
LIBS_DEP_DIR = $(addprefix libs/, $(LIBS_DEP))
ROOTFSIMG_DIR = $(abspath rootfsimg)
UTILS_DIR = $(abspath utils)
NETWORK ?=
NETWORK_DIR = $(abspath network)
DASICS_MODULES_DIR = $(ROOTFSIMG_DIR)/root/modules/dasics
OLD_DASICS_MODULE_FILES = utmod.ko tmod.ko \
	a2_utmod.ko a2_tmod.ko \
	c3_policy_test.ko c4_bound_table_test.ko c5_call_transaction_test.ko \
	d1_frame_state_test.ko \
	d3_utmod.ko d3_tmod.ko d4_utmod.ko d4_tmod.ko \
	e1_utmod.ko e1_tmod.ko e5_utmod.ko e5_tmod.ko \
	f1_utmod.ko f1_tmod.ko
OLD_DASICS_MODULE_PATHS = $(addprefix $(ROOTFSIMG_DIR)/root/, \
	$(OLD_DASICS_MODULE_FILES))

ROOTFSIMG_NEW_DIRS = bin dev dev/pts lib proc sbin sys tmp mnt root \
	usr usr/bin usr/sbin usr/lib var var/run

$(shell cd $(ROOTFSIMG_DIR) && mkdir -p $(ROOTFSIMG_NEW_DIRS))

.DEFAULT_GOAL = all

.PHONY: init all prepare-profile $(APPS_DIR) $(LIBS_DIR) $(LIBS_DEP_DIR) \
	network initramfs clean repoclean distclean

init:
	git submodule update --init --depth 1
	@$(foreach dir,$(APPS_DIR) $(LIBS_DIR) $(LIBS_DEP_DIR), \
		$(if $(wildcard $(dir)/repo), \
			$(if $(wildcard $(dir)/patchfile.patch), \
				echo "Applying patch to $(dir)/repo"; \
				git -C "$(dir)/repo" apply "../patchfile.patch"; \
			) \
		) \
	)

all: prepare-profile $(APPS_DIR) network
	$(MAKE) -s -C $(RISCV_ROOTFS_HOME) initramfs

prepare-profile:
	rm -rf $(DASICS_MODULES_DIR)
	rm -f $(OLD_DASICS_MODULE_PATHS)
	mkdir -p $(DASICS_MODULES_DIR)

$(APPS_DIR): %: $(LIBS_DIR) $(LIBS_DEP_DIR) | prepare-profile
	$(MAKE) -s -C $@ install

apps/dasics-test/a2-tmod: apps/dasics-test/a2-utmod
apps/dasics-test/tmod: apps/dasics-test/utmod
apps/dasics-test/d3-tmod: apps/dasics-test/d3-utmod
apps/dasics-test/d4-tmod: apps/dasics-test/d4-utmod
apps/dasics-test/e1-tmod: apps/dasics-test/e1-utmod
apps/dasics-test/e5-tmod: apps/dasics-test/e5-utmod
apps/dasics-test/f1-tmod: apps/dasics-test/f1-utmod
apps/dasics-test/dummy-logic: apps/dasics-test/dummy-glue

$(LIBS_DIR): %:
	$(MAKE) -s -C $@ install

network: | prepare-profile
	$(MAKE) -s -C $(NETWORK_DIR) NETWORK=$(NETWORK)

initramfs:
	python $(UTILS_DIR)/gen_initramfs.py

clean:
	$(MAKE) -s -C $(NETWORK_DIR) clean
	$(foreach dir, $(LIBS_DIR) $(LIBS_DEP_DIR) $(APPS_DIR), $(MAKE) -s -C $(dir) clean ;)
	cd $(ROOTFSIMG_DIR) && rm -f initramfs*.txt && rm -rf $(ROOTFSIMG_NEW_DIRS)

repoclean: clean
	$(foreach dir, $(LIBS_DIR) $(LIBS_DEP_DIR) $(APPS_DIR), \
		$(if $(wildcard $(dir)/repo/Makefile), \
			$(MAKE) -s -C $(dir)/repo clean ;) \
	)

distclean: repoclean
	git submodule deinit -f --all
