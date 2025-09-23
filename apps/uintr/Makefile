NAME = uintr

TEST_DIR = $(APP_DIR)/test
LIBS_DIR = $(APP_DIR)/lib

override APP = $(DST_DIR)/uintr

include $(RISCV_ROOTFS_HOME)/Makefile.app

SRCS = $(TEST_DIR)/uintr.c 

LIBS = lib/uintr_entry.S 
# cross compile
CROSS_COMPILE = riscv64-unknown-elf-

AS = $(CROSS_COMPILE)gcc
CC = $(CROSS_COMPILE)gcc
CXX = $(CROSS_COMPILE)g++
LD = $(CROSS_COMPILE)gcc

# compile option
# Note: gcc -O2 option and RVC will cause 
# some unexpected compilation results.

INCLUDES  = $(addprefix -I, ${INC_DIR})

CFLAGS   += -O0 -march=rv64imad -MMD $(INCLUDES) -g -static
CXXFLAGS += -O0 -march=rv64imad -MMD $(INCLUDES) -g -static

# Files to be compiled
LIBOBJS = $(addprefix $(DST_DIR)/, $(addsuffix .o, $(basename $(LIBS))))

# Compilation patterns
$(DST_DIR)/lib/%.o: $(LIBS_DIR)/%.c 
	echo + CC $^
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $^
$(DST_DIR)/%.o: $(TEST_DIR)/%.c 
	echo + CC $^
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $^
$(DST_DIR)/lib/%.o: $(LIBS_DIR)/%.S   
	echo + CC $^
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $^

$(APP):%:%.o $(LIBOBJS)
	@echo + LD $@
	@$(LD) $(LDFLAGS) -o $@ $^

# Dependencies
DEPS = $(addprefix $(DST_DIR)/, $(addsuffix .d, $(basename $(SRCS))))
-include $(DEPS)
