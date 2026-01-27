
NAME    = cdma_demo
SRCS    = axicdma_mmio_demo.c
IST_DIR = $(abspath $(RISCV_ROOTFS_HOME)/rootfsimg/bin)
DASICS_SUPPORT = $(abspath $(RISCV_ROOTFS_HOME)/libs/LibDASICS)
INC_DIR += $(DASICS_SUPPORT)/repo/include
LDFLAGS += -L$(DASICS_SUPPORT)/build -lDASICS -T$(DASICS_SUPPORT)/repo/ld.lds

ifneq ($(RISCV_ROOTFS_HOME),)
include $(RISCV_ROOTFS_HOME)/Makefile.compile
else
$(error RISCV_ROOTFS_HOME is not defined)
endif
