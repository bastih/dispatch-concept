#ifndef __DEBUG_HPP
#define __DEBUG_HPP

#include <iostream>



#ifndef NDEBUG

template <typename T>
void print(T arg, std::string delim=" ") {
  std::cout << arg << delim;
}

template <typename T>
void print(std::vector<T> arg, std::string delim=" ") {
  std::cout << "[";
  for(const auto& val : arg) { print(val, ", "); };
  std::cout << "]";
}

template<typename T>
void debug(T arg) {
  print(arg, "");
  std::cout << "\n";
}

template <typename T, typename... ARGS>
void debug(T arg, ARGS... args) {
  print(arg);
  debug(args...);
}

#else
template <typename... ARGS>
inline void debug(ARGS...) {};
#endif


#endif
