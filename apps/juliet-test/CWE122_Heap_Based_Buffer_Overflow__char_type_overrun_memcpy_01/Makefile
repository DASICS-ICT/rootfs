NAME = CWE122_Heap_Based_Buffer_Overflow__char_type_overrun_memcpy_01

JULIET_SUPPORT = $(abspath ../testcasesupport)
INC_DIR += $(JULIET_SUPPORT)
CFLAGS += -DINCLUDEMAIN -DOMITGOOD
LDFLAGS += -lpthread

SRCS = $(NAME).c $(wildcard $(JULIET_SUPPORT)/*.c)

ifneq ($(RISCV_ROOTFS_HOME),)
include $(RISCV_ROOTFS_HOME)/Makefile.compile
else
$(error RISCV_ROOTFS_HOME is not defined)
endif