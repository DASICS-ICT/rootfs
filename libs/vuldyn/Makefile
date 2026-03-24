# Makefile for libvuldyn.so -- DASICS dynamic library test.
#
# Compiles vuldyn.c and vuldyn_syms.S into a position-independent shared
# library.  The .so is installed into rootfsimg/usr/lib so the dynamic
# linker can find it at runtime on the target system.
#
# IMPORTANT: We deliberately do NOT use -fno-plt here.  External
# function calls (e.g. dasics_umaincall) go through PLT stubs,
# which requires each compartment to have PLT jump bounds and
# GOT mem bounds in its DASICS configuration.

NAME = libvuldyn.so

# LibDASICS provides the DASICS headers (udasics.h, fit.h, etc.)
DASICS_SUPPORT = $(abspath $(RISCV_ROOTFS_HOME)/libs/LibDASICS)
INC_DIR += $(DASICS_SUPPORT)/repo/include

ifneq ($(RISCV_ROOTFS_HOME),)
include $(RISCV_ROOTFS_HOME)/Makefile.app
else
$(error RISCV_ROOTFS_HOME is not defined)
endif

CROSS_COMPILE ?= riscv64-unknown-linux-gnu-
CC      = $(CROSS_COMPILE)gcc
OBJDUMP = $(CROSS_COMPILE)objdump

# -O0: no optimisation (predictable instruction layout for debugging)
# -fPIC: position-independent code (required for shared libraries)
# NOTE: no -fno-plt -- external calls go through PLT stubs
CFLAGS = -O0 -g -fPIC $(addprefix -I, $(INC_DIR))

# Install the .so into rootfsimg/usr/lib (overrides the default IST_DIR
# which targets rootfsimg/root for executables)
override IST_DIR = $(abspath $(RISCV_ROOTFS_HOME)/rootfsimg/usr/lib)

# Source files:
#   vuldyn.c       -- untrusted library code (compartment functions)
#   vuldyn_syms.S  -- exported boundary address data objects (.quad wrappers
#                     around internal linker-script symbols, see file header
#                     for details on why this indirection is necessary)
SRCS = $(APP_DIR)/vuldyn.c $(APP_DIR)/vuldyn_syms.S

# Build rule: compile and link as a shared library.
#   -T ld_vuldyn.lds:    custom linker script defining section boundaries
#   --version-script:    controls which symbols are exported to the dynamic
#                        symbol table (boundary data objects + functions)
$(APP): $(SRCS) $(APP_DIR)/ld_vuldyn.lds $(APP_DIR)/vuldyn.ver
	@echo + CC libvuldyn.so
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -shared -o $@ $(SRCS) -T$(APP_DIR)/ld_vuldyn.lds \
		-Wl,--version-script=$(APP_DIR)/vuldyn.ver
	@echo + OBJDUMP $@
	$(OBJDUMP) -d $@ > $(DST_DIR)/vuldyn.txt
