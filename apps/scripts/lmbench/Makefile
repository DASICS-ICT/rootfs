APP_NAME := lmbench
REPO_PATH := ../../$(APP_NAME)

TARGET_FILE := $(INITRAMFS_ROOT)/lib/libtirpc.so

CFLAGS += -O
CFLAGS += -I$(INITRAMFS_ROOT)/include/tirpc
LDFLAGS += -L$(INITRAMFS_ROOT)/lib
LDLIBS += -ltirpc
CC = $(CROSS_COMPILE)gcc
AR = $(CROSS_COMPILE)ar
RANLIB = $(CROSS_COMPILE)ranlib
override OS = unknown

export CFLAGS
export LDFLAGS
export LDLIBS
export CC
export AR
export RANLIB
export OS

.PHONY: all clean

all:
	mkdir -p $(REPO_PATH)/bin/unknown
	cd $(REPO_PATH)/src && ../scripts/build all
	$(MAKE) -C $(REPO_PATH)/src BASE=$(INITRAMFS_ROOT) install-target

clean:
	$(MAKE) -C $(REPO_PATH) clean
