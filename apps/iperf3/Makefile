NAME    = iperf3
IST_DIR = $(abspath $(RISCV_ROOTFS_HOME)/rootfsimg/bin)

ifneq ($(RISCV_ROOTFS_HOME),)
include $(RISCV_ROOTFS_HOME)/Makefile.app
else
$(error RISCV_ROOTFS_HOME is not defined)
endif

CROSS_COMPILE = riscv64-unknown-linux-gnu

IPERF3_REPO   = $(abspath repo)
IPERF3_ELF    = $(IPERF3_REPO)/src/iperf3

.PHONY: iperf3

iperf3: $(IPERF3_CONFIG)
	$(MAKE) -C $(IPERF3_REPO)

$(IPERF3_CONFIG): 
	cd $(IPERF3_REPO) && ./configure --host=$(CROSS_COMPILE) --enable-static-bin --without-openssl

$(IPERF3_ELF): iperf3

$(APP): $(IPERF3_ELF) 

install:: $(APP) 
	ln -sf $(abspath $(IPERF3_ELF)) $(abspath $(RISCV_ROOTFS_HOME)/rootfsimg/bin/iperf3)