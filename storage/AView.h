#pragma once

#include "dispatch2/Base.h"

class AView : Base {
};

class RangeViewIterator : std::iterator<std::forward_iterator_tag, size_t> {
    size_t _value;
 public:
    RangeViewIterator(size_t v) : _value(v) {}
    RangeViewIterator& operator++() { ++_value; return *this; }
    RangeViewIterator operator++(int) { RangeViewIterator old(*this); operator++(); return old; } //postfix
    size_t operator*() { return _value; }
    bool operator==(const RangeViewIterator& other) { return _value == other._value; }
};

class RangeView : public AView {
    size_t _from, _to;

 public:
 RangeView(size_t _from, size_t _to) : _from(from), _to(to) {}
 RangeViewIterator begin() const {
     return RangeViewIterator(_from);
 }

 RangeViewIterator end() const {
     return RangeViewIterator(_to);
 }

};

class ListView : public AView {
    std::vector<std::size_t> offsets;
 public:
    decltype(std::begin(offsets)) begin() const {
        return std::begin(offsets);
    }

    decltype(std::end(offsets)) begin() const {
        return std::end(offsets);
    }

};
