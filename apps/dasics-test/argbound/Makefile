NAME = argbound
SRCS = $(NAME).c fit_generated.c

DASICS_SUPPORT = $(abspath $(RISCV_ROOTFS_HOME)/libs/LibDASICS)
INC_DIR += $(DASICS_SUPPORT)/repo/include

ifneq ($(RISCV_ROOTFS_HOME),)
include $(RISCV_ROOTFS_HOME)/Makefile.compile
else
$(error RISCV_ROOTFS_HOME is not defined)
endif

LDFLAGS += -L$(DASICS_SUPPORT)/build -lDASICS -T$(APP_DIR)/ld.lds