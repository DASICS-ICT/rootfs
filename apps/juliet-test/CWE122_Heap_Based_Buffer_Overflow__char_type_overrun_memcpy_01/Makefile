NAME = CWE122_Heap_Based_Buffer_Overflow__char_type_overrun_memcpy_01

JULIET_SUPPORT = $(abspath ../testcasesupport)
DASICS_SUPPORT = $(abspath $(RISCV_ROOTFS_HOME)/libs/LibDASICS)
UTHASH_SUPPORT = $(abspath $(RISCV_ROOTFS_HOME)/includes/uthash)
INC_DIR += $(JULIET_SUPPORT) $(DASICS_SUPPORT)/repo/include $(UTHASH_SUPPORT)/include
CFLAGS += -DINCLUDEMAIN -DOMITGOOD
LDFLAGS += -lpthread -L$(DASICS_SUPPORT)/build -lDASICS -T$(DASICS_SUPPORT)/repo/ld.lds

SRCS = $(NAME).c $(wildcard $(JULIET_SUPPORT)/*.c)

ifneq ($(RISCV_ROOTFS_HOME),)
include $(RISCV_ROOTFS_HOME)/Makefile.compile
else
$(error RISCV_ROOTFS_HOME is not defined)
endif