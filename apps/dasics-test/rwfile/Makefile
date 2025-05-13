NAME = rwfile
SRCS = $(NAME).c
PWD := $(shell pwd)
ARCH ?= riscv
CROSS_COMPILE ?= riscv64-unknown-linux-gnu-
ROOTFSIMG := ~/dasics/rootfs/rootfsimg/root
BUILD_DIR := $(PWD)/build
FLAGS ?= -fPIC -g
ccflags-y += $(FLAGS)

install:
	mkdir $(BUILD_DIR)
	$(CROSS_COMPILE)gcc $(ccflags-y) -o $(BUILD_DIR)/$(NAME) $(SRCS)
	ln -sf $(BUILD_DIR)/$(NAME) $(ROOTFSIMG)/$(NAME)
	
clean:
	rm -rf $(BUILD_DIR)
