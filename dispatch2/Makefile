dispatch2 := $(realpath $(dir $(lastword $(MAKEFILE_LIST))))

-include ../rules.mk

dispatch2.libname := dispatch2
dispatch2.includes := $(PROJECT_ROOT)
$(eval $(call library,dispatch2))
