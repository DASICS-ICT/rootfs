obj-m += d3_tmod.o
ccflags-y += -fPIC -g

MODULE_NAME := d3_tmod
PEER_MODULE := d3-utmod
MODULE_DIR := $(abspath $(if $(src),$(src),$(CURDIR)))
include $(MODULE_DIR)/../module-test.mk
