NAME    = libsqlite3.so.0
IST_DIR = $(abspath $(RISCV_ROOTFS_HOME)/rootfsimg/usr/lib)
APP     = $(DST_DIR)/lib/$(NAME)

ifneq ($(RISCV_ROOTFS_HOME),)
include $(RISCV_ROOTFS_HOME)/Makefile.app
else
$(error RISCV_ROOTFS_HOME is not defined)
endif

SQLITE_REPO   = $(abspath repo)
SQLITE_MAKEFILE = $(SQLITE_REPO)/Makefile

.PHONY: sqlite

sqlite: $(SQLITE_MAKEFILE)
	$(MAKE) -C $(SQLITE_REPO)

$(SQLITE_MAKEFILE):
	cd $(SQLITE_REPO) && ./configure --host=riscv64-unknown-linux-gnu \
		--prefix=$(DST_DIR) --disable-tcl --disable-readline

$(APP): sqlite
	$(MAKE) -C $(SQLITE_REPO) install

install:: $(APP)
	@mkdir -p $(IST_DIR)
	@ln -sf $(DST_DIR)/lib/libsqlite3.so $(IST_DIR)/libsqlite3.so
	@ln -sf $(DST_DIR)/lib/libsqlite3.so.0 $(IST_DIR)/libsqlite3.so.0
	@cp -P $(DST_DIR)/lib/libsqlite3.so.0.* $(IST_DIR)/ || true
