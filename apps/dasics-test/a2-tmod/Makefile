obj-m += a2_tmod.o
a2_tmod-y := module.o calls.o
ccflags-y += -fPIC -g

MODULE_NAME := a2_tmod
PEER_MODULE := a2-utmod
MODULE_DIR := $(abspath $(if $(src),$(src),$(CURDIR)))
include $(MODULE_DIR)/../module-test.mk
