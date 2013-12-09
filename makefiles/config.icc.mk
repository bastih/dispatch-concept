CC := ~/intel/bin/icc
CXX:= ~/intel/bin/icpc -gxx-name=g++-4.8
LD := ~/intel/bin/icpc -gxx-name=g++-4.8
AR := ~/intel/bin/xiar

COMMON_FLAGS += -include=/usr/include/x86_64-linux-gnu/c++/4.8/bits/c++config.h -include=/usr/include/x86_64-linux-gnu/c++/4.8/bits/c++allocator.h -opt-prefetch -xhost -I/usr/include/x86_64-linux-gnu/c++/4.8/ -pedantic -Wall -Wextra -fast
LDFLAGS += -dynamic-linker=/usr/bin/ld.bfd
LIBS += stdc++
