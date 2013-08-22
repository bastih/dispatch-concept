#!/bin/sh

clang++ *.cpp -O2  -fcolor-diagnostics --std=c++11 -Wall -Wextra -Weverything -Wno-global-constructors -Wno-c++98-compat 2>&1 | sed -e 's/mpl_::na.*mpl_::na/...mpl_::na.../g' -e "s/boost::mpl::/mpl::/g" -e "s/::/:/g" -s
