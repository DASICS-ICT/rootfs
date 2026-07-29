obj-m += d4_tmod.o
ccflags-y += -fPIC -g

MODULE_NAME := d4_tmod
PEER_MODULE := d4-utmod
MODULE_DIR := $(abspath $(if $(src),$(src),$(CURDIR)))
include $(MODULE_DIR)/../module-test.mk
