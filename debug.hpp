#pragma once

#include <iostream>
#include <iomanip>
#include <vector>

template <typename T>
void print(T arg, std::string delim=" ") {
  std::cout << arg << delim;
}

inline void print(long long arg, std::string delim) {
  std::cout << std::setw(12) << arg << delim;
}

inline void print(unsigned long long arg, std::string delim) {
  std::cout << std::setw(12) << arg << delim;
}

inline void print(std::uint64_t arg, std::string delim) {
  std::cout << std::setw(12) << arg << delim;
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
  print(arg, " ");
  debug(args...);
}

