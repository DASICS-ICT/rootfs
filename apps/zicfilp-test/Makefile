NAME    = zicfilp-test
IST_DIR = $(abspath $(RISCV_ROOTFS_HOME)/rootfsimg/root)
NEED_INSTALL = false

ifneq ($(RISCV_ROOTFS_HOME),)
include $(RISCV_ROOTFS_HOME)/Makefile.app
else
$(error RISCV_ROOTFS_HOME is not defined)
endif

# Source files
SRC_DIR = src
SRCS = $(wildcard $(SRC_DIR)/*.c)
PROGS = $(patsubst $(SRC_DIR)/%.c, $(DST_DIR)/%, $(SRCS))

# Compiler settings
CC = $(CROSS_COMPILE)gcc
# Use -O1 to avoid aggressive optimizations that might reorder code
# Use -fno-reorder-functions to preserve function order
# Use -fno-align-functions to prevent automatic function alignment padding
CFLAGS = -Wall -O1 -static -fno-reorder-functions -fno-align-functions

# Build all test programs
.PHONY: all_tests

all_tests: $(PROGS)

$(DST_DIR)/%: $(SRC_DIR)/%.c
	@mkdir -p $(DST_DIR)
	$(CC) $(CFLAGS) $< -o $@
	@echo "Built: $@"

# Dummy target for Makefile.app
$(APP):
	@echo "Building zicfilp tests..."
	@$(MAKE) -s all_tests

# Install all programs
install:: all_tests
	@mkdir -p $(IST_DIR)
	@mkdir -p $(RISCV_ROOTFS_HOME)/rootfsimg/build
	@for prog in $(PROGS); do \
		ln -sf $$(readlink -f $$prog) $(IST_DIR)/$$(basename $$prog); \
		echo "Installed: $$(basename $$prog) -> $(IST_DIR)"; \
		cp -f $$prog $(RISCV_ROOTFS_HOME)/rootfsimg/build/$$(basename $$prog); \
		echo "Copied: $$(basename $$prog) -> rootfsimg/build/"; \
	done
