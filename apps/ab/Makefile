NAME = ab
SRCS = $(NAME).c

LIBS_DIR      = $(abspath $(RISCV_ROOTFS_HOME)/libs)
APR_BUILD     = $(LIBS_DIR)/apr/build
APRUTIL_BUILD = $(LIBS_DIR)/apr-util/build

INC_DIR += . $(APR_BUILD)/include/apr-1 $(APRUTIL_BUILD)/include/apr-1
CFLAGS  += -D_REENTRANT -D_GNU_SOURCE -D_LARGEFILE64_SOURCE
LDFLAGS += -L$(APR_BUILD)/lib -L$(APRUTIL_BUILD)/lib \
	-lapr-1 -laprutil-1 -lm -lpthread

ifneq ($(RISCV_ROOTFS_HOME),)
include $(RISCV_ROOTFS_HOME)/Makefile.compile
else
$(error RISCV_ROOTFS_HOME is not defined)
endif