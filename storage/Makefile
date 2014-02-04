storage := $(realpath $(dir $(lastword $(MAKEFILE_LIST))))

-include ../rules.mk
include $(PROJECT_ROOT)/dispatch2/Makefile

storage.libname := storage
storage.deps := dispatch2
storage.includes := $(PROJECT_ROOT)
$(eval $(call library,storage))
