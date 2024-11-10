NAME = redis-server

ifneq ($(RISCV_ROOTFS_HOME),)
include $(RISCV_ROOTFS_HOME)/Makefile.app
else
$(error RISCV_ROOTFS_HOME is not defined)
endif

CROSS_COMPILE ?= riscv64-unknown-linux-gnu-

REDIS_REPO = $(abspath repo)
REDIS_CONFIG = $(REDIS_REPO)/redis.conf
REDIS_SRV = $(REDIS_REPO)/src/redis-server
REDIS_CLI = $(REDIS_REPO)/src/redis-cli
REDIS_BENCH = $(REDIS_REPO)/src/redis-benchmark

.PHONY: redis

redis:
	$(MAKE) -C $(REDIS_REPO) CC=$(CROSS_COMPILE)gcc CXX=$(CROSS_COMPILE)g++ MALLOC=libc

$(REDIS_SRV): redis

$(APP): $(REDIS_SRV)
	ln -sf $(abspath $<) $@

# redis-server is installed in Makefile.app, thus we install redis.conf and redis-cli here
install:: $(APP)
	@ln -sf $(abspath $(REDIS_CONFIG)) $(IST_DIR)/redis.conf
	@ln -sf $(abspath $(REDIS_CLI)) $(IST_DIR)/redis-cli
	@ln -sf $(abspath $(REDIS_BENCH)) $(IST_DIR)/redis-benchmark