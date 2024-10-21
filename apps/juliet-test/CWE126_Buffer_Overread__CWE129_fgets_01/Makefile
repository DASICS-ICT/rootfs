NAME = CWE126_Buffer_Overread__CWE129_fgets_01

JULIET_SUPPORT = $(abspath ../testcasesupport)
INC_DIR += $(JULIET_SUPPORT)
CFLAGS += -DINCLUDEMAIN
LDFLAGS += -lpthread

SRCS = $(NAME).c $(wildcard $(JULIET_SUPPORT)/*.c)

include $(RISCV_ROOTFS_HOME)/Makefile.compile

override IST_DIR = $(abspath $(RISCV_ROOTFS_HOME)/rootfsimg/root)