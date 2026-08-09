APP_NAME := libtirpc
REPO_PATH := ../../$(APP_NAME)

TARGET_FILE := $(INITRAMFS_ROOT)/lib/libtirpc.so

CC = $(CROSS_COMPILE)gcc
CXX = $(CROSS_COMPILE)g++
LD = $(CROSS_COMPILE)ld
AR = $(CROSS_COMPILE)ar
NM = $(CROSS_COMPILE)nm
RANLIB = $(CROSS_COMPILE)ranlib
STRIP = $(CROSS_COMPILE)strip
OBJDUMP = $(CROSS_COMPILE)objdump

export CC
export CXX
export LD
export AR
export NM
export RANLIB
export STRIP
export OBJDUMP

.PHONY: all clean

all: $(TARGET_FILE)

$(TARGET_FILE):
	cd $(REPO_PATH) && \
		./configure --prefix=/ --host=riscv64-linux-gnu --disable-gssapi
	$(MAKE) -C $(REPO_PATH)
	$(MAKE) -C $(REPO_PATH) DESTDIR=$(INITRAMFS_ROOT) install

clean:
	$(MAKE) -C $(REPO_PATH) clean
