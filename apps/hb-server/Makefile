NAME = hb-server
SRCS = hb-server.c

OPENSSL_INSTALL = $(abspath $(RISCV_ROOTFS_HOME)/libs/openssl/build)
INC_DIR += $(OPENSSL_INSTALL)/include
LDFLAGS += -L$(OPENSSL_INSTALL)/lib -lssl -lcrypto -ldl

ifneq ($(RISCV_ROOTFS_HOME),)
include $(RISCV_ROOTFS_HOME)/Makefile.compile
else
$(error RISCV_ROOTFS_HOME is not defined)
endif