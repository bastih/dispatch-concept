#ifndef __DEBUG_HPP
#define __DEBUG_HPP

#include <iostream>

#ifndef NDEBUG 

template<typename T>
void debug(T arg) {
  std::cout << arg << std::endl;
}

template <typename T, typename... ARGS>
void debug(T arg, ARGS... args) {
  std::cout << arg << " ";
  debug(args...);
}

#else
template <typename... ARGS>
inline void debug(ARGS...) {};

#endif


#endif
