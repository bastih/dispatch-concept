gtest := $(realpath $(dir $(lastword $(MAKEFILE_LIST))))

-include ../../rules.mk

gtest.libname := gtest
gtest.cpps := $(gtest)/gtest/gtest-all.cc
gtest.includes := $(gtest)
$(eval $(call library,gtest))
