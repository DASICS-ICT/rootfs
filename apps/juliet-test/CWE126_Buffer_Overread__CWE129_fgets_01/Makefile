NAME = CWE126_Buffer_Overread__CWE129_fgets_01

JULIET_SUPPORT = $(abspath ../testcasesupport)
DASICS_SUPPORT = $(abspath $(RISCV_ROOTFS_HOME)/libs/LibDASICS)
INC_DIR += $(JULIET_SUPPORT) $(DASICS_SUPPORT)/repo/include
CFLAGS += -DINCLUDEMAIN -DOMITGOOD
LDFLAGS += -lpthread -L$(DASICS_SUPPORT)/build -lDASICS -T$(DASICS_SUPPORT)/repo/ld.lds

SRCS = $(NAME).c $(wildcard $(JULIET_SUPPORT)/*.c)

include $(RISCV_ROOTFS_HOME)/Makefile.compile