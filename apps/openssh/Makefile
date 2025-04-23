NAME    = ssh
IST_DIR = $(abspath $(RISCV_ROOTFS_HOME)/rootfsimg/usr/local/bin)

ifneq ($(RISCV_ROOTFS_HOME),)
include $(RISCV_ROOTFS_HOME)/Makefile.app
else
$(error RISCV_ROOTFS_HOME is not defined)
endif

CROSS_COMPILE = riscv64-unknown-linux-gnu

LIBS_DIR       = $(abspath $(RISCV_ROOTFS_HOME)/libs)
ZLIB_DIR       = $(LIBS_DIR)/zlib/build
OPENSSL_DIR    = $(LIBS_DIR)/openssl/build

OPENSSH_REPO   = $(abspath repo)
OPENSSH_CONFIG = $(OPENSSH_REPO)/config.h
OPENSSH_ELF    = $(OPENSSH_REPO)/ssh

.PHONY: openssh

openssh: $(OPENSSH_CONFIG)
	$(MAKE) -C $(OPENSSH_REPO)
	$(CROSS_COMPILE)-strip $(OPENSSH_ELF)
	$(CROSS_COMPILE)-strip $(OPENSSH_REPO)/sshd
	$(CROSS_COMPILE)-strip $(OPENSSH_REPO)/scp
	$(CROSS_COMPILE)-strip $(OPENSSH_REPO)/ssh-add
	$(CROSS_COMPILE)-strip $(OPENSSH_REPO)/ssh-agent
	$(CROSS_COMPILE)-strip $(OPENSSH_REPO)/ssh-keygen
	$(CROSS_COMPILE)-strip $(OPENSSH_REPO)/ssh-keyscan
	$(CROSS_COMPILE)-strip $(OPENSSH_REPO)/sftp
	$(CROSS_COMPILE)-strip $(OPENSSH_REPO)/sshd-session
	$(CROSS_COMPILE)-strip $(OPENSSH_REPO)/sftp-server

$(OPENSSH_CONFIG):
	cd $(OPENSSH_REPO) && autoreconf && \
		./configure --host=$(CROSS_COMPILE) \
			--with-libs --with-zlib=$(ZLIB_DIR) --with-ssl-dir=$(OPENSSL_DIR) \
			--disable-etc-default-login

$(OPENSSH_ELF): openssh
	# FIXME: Deleting this comment will cause unexpected compiler errors.

$(APP): $(OPENSSH_ELF)
	@ln -sf $(abspath $<) $@

install:: $(APP)
	@ln -sf $(OPENSSH_REPO)/sshd $(RISCV_ROOTFS_HOME)/rootfsimg/usr/local/sbin/sshd
	@ln -sf $(OPENSSH_REPO)/scp $(IST_DIR)/scp
	@ln -sf $(OPENSSH_REPO)/ssh-add $(IST_DIR)/ssh-add
	@ln -sf $(OPENSSH_REPO)/ssh-agent $(IST_DIR)/ssh-agent
	@ln -sf $(OPENSSH_REPO)/ssh-keygen $(IST_DIR)/ssh-keygen
	@ln -sf $(OPENSSH_REPO)/ssh-keyscan $(IST_DIR)/ssh-keyscan
	@ln -sf $(OPENSSH_REPO)/sftp $(IST_DIR)/sftp
	@ln -sf $(OPENSSH_REPO)/sshd-session $(RISCV_ROOTFS_HOME)/rootfsimg/usr/local/libexec/sshd-session
	@ln -sf $(OPENSSH_REPO)/sftp-server $(RISCV_ROOTFS_HOME)/rootfsimg/usr/local/libexec/sftp-server
