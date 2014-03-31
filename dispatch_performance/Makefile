dp1 := $(realpath $(dir $(lastword $(MAKEFILE_LIST))))
dp2 := $(realpath $(dir $(lastword $(MAKEFILE_LIST))))

-include ../rules.mk

include $(PROJECT_ROOT)/dispatch2/Makefile

dp1.binname := dp1
dp1.cpps := $(dp1)/dispatch1.cpp
dp1.deps := dispatch2
$(eval $(call full_link_binary,dp1))

dp2.binname := dp2
dp2.cpps := $(dp1)/dispatch2.cpp
dp2.deps := dispatch2
$(eval $(call full_link_binary,dp2))
