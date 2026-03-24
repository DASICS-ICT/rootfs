# Makefile for the vuldyn test application.
#
# This builds the trusted main program that links against:
#   - libvuldyn.so  (the untrusted dynamic library under test)
#   - libDASICS.a   (static DASICS/FIT support library)
#   - libmimalloc   (self-managed heap allocator)
#
# The executable is installed to rootfsimg/root for inclusion in initramfs.

NAME = vuldyn
SRCS = main.c fit_generated.c

# Path to LibDASICS (provides udasics.h, fit.h, compartment.h, etc.)
DASICS_SUPPORT = $(abspath $(RISCV_ROOTFS_HOME)/libs/LibDASICS)

# Path to mimalloc (provides mi_malloc/mi_free for self-managed heaps)
MIMALLOC_SUPPORT = $(abspath $(RISCV_ROOTFS_HOME)/libs/mimalloc)

# Path to libvuldyn.so (the dynamic library we are testing)
VULDYN_SUPPORT = $(abspath $(RISCV_ROOTFS_HOME)/libs/vuldyn)

# Include directories for headers
INC_DIR += $(DASICS_SUPPORT)/repo/include
INC_DIR += $(MIMALLOC_SUPPORT)/repo/include

ifneq ($(RISCV_ROOTFS_HOME),)
include $(RISCV_ROOTFS_HOME)/Makefile.compile
else
$(error RISCV_ROOTFS_HOME is not defined)
endif

# Linker flags:
#   -lDASICS:   static DASICS/FIT library
#   -T ld.lds:  custom linker script for main executable (defines PLT/GOT/text bounds)
#   -lmimalloc: mimalloc for compartment heap isolation
#   -lvuldyn:   the dynamic library under test
#   -rpath:     embed runtime library search path so the dynamic linker
#               finds libvuldyn.so at /usr/lib on the target filesystem
LDFLAGS += -L$(DASICS_SUPPORT)/build -lDASICS -T$(APP_DIR)/ld.lds
LDFLAGS += -L$(MIMALLOC_SUPPORT)/build/lib -lmimalloc
LDFLAGS += -L$(VULDYN_SUPPORT)/build -lvuldyn
LDFLAGS += -Wl,-rpath,/usr/lib
