obj-m += e1_tmod.o
ccflags-y += -fPIC -g

MODULE_NAME := e1_tmod
PEER_MODULE := e1-utmod
MODULE_DIR := $(abspath $(if $(src),$(src),$(CURDIR)))
include $(MODULE_DIR)/../module-test.mk
