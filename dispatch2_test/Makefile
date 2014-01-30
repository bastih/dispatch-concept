test2 := $(realpath $(dir $(lastword $(MAKEFILE_LIST))))

-include ../rules.mk

include $(PROJECT_ROOT)/dispatch2/Makefile
include $(PROJECT_ROOT)/thirdparty/gtest17/Makefile

test2.binname := test2
test2.deps := dispatch2 gtest
$(eval $(call test-binary,test2))
