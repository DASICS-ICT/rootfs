NAME = libDASICS.a

ifneq ($(RISCV_ROOTFS_HOME),)
include $(RISCV_ROOTFS_HOME)/Makefile.app
else
$(error RISCV_ROOTFS_HOME is not defined)
endif

LIBDASICS_REPO = $(abspath repo)
LIBDASICS_A    = $(LIBDASICS_REPO)/build/libDASICS.a

override IST_DIR = $(abspath $(RISCV_ROOTFS_HOME)/rootfsimg/usr/lib)

.PHONY: libDASICS

libDASICS:
	$(MAKE) -C $(LIBDASICS_REPO)

$(LIBDASICS_A): libDASICS

$(APP): $(LIBDASICS_A)
	ln -sf $(abspath $<) $@