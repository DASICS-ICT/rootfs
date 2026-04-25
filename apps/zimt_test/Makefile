NAME = zimt_test
SRCS = zimt_test.c

INC_DIR += .

ifneq ($(RISCV_ROOTFS_HOME),)
include $(RISCV_ROOTFS_HOME)/Makefile.compile
else
$(error RISCV_ROOTFS_HOME is not defined)
endif
