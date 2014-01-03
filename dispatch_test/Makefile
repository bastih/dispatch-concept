test := $(realpath $(dir $(lastword $(MAKEFILE_LIST))))

-include ../rules.mk

include $(PROJECT_ROOT)/dispatch/Makefile
include $(PROJECT_ROOT)/thirdparty/gtest17/Makefile

test.binname := test
test.deps := dispatch gtest
$(eval $(call test-binary,test))
