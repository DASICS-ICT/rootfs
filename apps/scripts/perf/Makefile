APP_NAME := perf
REPO_PATH := ../../$(APP_NAME)

TARGET_FILE := $(INITRAMFS_ROOT)/bin/perf

CC = $(CROSS_COMPILE)gcc

.PHONY: all clean

all: $(TARGET_FILE)

$(TARGET_FILE): $(REPO_PATH)/main.c $(REPO_PATH)/read_hpm.c $(REPO_PATH)/write_hpm.c
	mkdir -p $(dir $@)
	$(CC) $^ -o $@

clean:
