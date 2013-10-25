storage := $(realpath $(dir $(lastword $(MAKEFILE_LIST))))

-include ../rules.mk

storage.libname := storage
storage.includes := $(PROJECT_ROOT)
$(eval $(call library,storage))
