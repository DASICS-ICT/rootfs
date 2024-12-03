NAME = curl-client
SRCS = $(NAME).c

LIBCURL_INSTALL = $(abspath $(RISCV_ROOTFS_HOME)/libs/curl/build)
INC_DIR += $(LIBCURL_INSTALL)/include
LDFLAGS += -L$(LIBCURL_INSTALL)/lib -lcurl

ifneq ($(RISCV_ROOTFS_HOME),)
include $(RISCV_ROOTFS_HOME)/Makefile.compile
else
$(error RISCV_ROOTFS_HOME is not defined)
endif