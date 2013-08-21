#!/bin/sh

clang++ -time *.cpp -O3  -fcolor-diagnostics --std=c++11 -Wall -Wextra -Wno-c++98-compat 2>&1 | sed -e 's/mpl_::na.*mpl_::na/...mpl_::na.../g' -e "s/boost::mpl::/mpl::/g" -e "s/::/:/g" -s
