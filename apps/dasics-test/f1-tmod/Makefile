obj-m += f1_tmod.o
ccflags-y += -fPIC -g

MODULE_NAME := f1_tmod
PEER_MODULE := f1-utmod
MODULE_DIR := $(abspath $(if $(src),$(src),$(CURDIR)))
include $(MODULE_DIR)/../module-test.mk
