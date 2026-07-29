obj-m += e5_tmod.o
ccflags-y += -fPIC -g

MODULE_NAME := e5_tmod
PEER_MODULE := e5-utmod
MODULE_DIR := $(abspath $(if $(src),$(src),$(CURDIR)))
include $(MODULE_DIR)/../module-test.mk
