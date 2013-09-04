#pragma once

#include <cstdint>
#include <iterator>
class Range {
 public:
  class RangeIter {
   public:
    using value_type = std::size_t;
    using difference_type = std::int64_t;
    using reference = value_type&;
    using pointer = value_type*;
    using iterator_category = std::forward_iterator_tag;

    explicit RangeIter(std::size_t val) : _value(val) {}

    inline bool operator==(const RangeIter& other) { return _value == other._value; }

    inline bool operator!=(const RangeIter& other) { return _value != other._value; }

    inline size_t operator*() { return _value; }

    inline void operator++() { _value++; }

    inline void operator--() { _value++; }

   private:
    std::size_t _value;
  };

  Range(std::size_t stop) : _stop(stop) {}
  Range(std::size_t start, std::size_t stop) : _start(start), _stop(stop) {}
  RangeIter begin() const { return RangeIter(_start); }
  RangeIter end() const { return RangeIter(_stop); }

  const std::size_t _start = 0;
  const std::size_t _stop;

 private:
};
