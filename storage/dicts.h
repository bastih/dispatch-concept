#pragma once

#include "storage/ADictionary.h"

#include <unordered_map>
#include <algorithm>
#include <vector>

#include "helpers/shortcuts.h"


template <typename T>
class OrderedDictionary final : public BaseDictionary<T> {
 public:
  std::size_t size() const { return _values.size(); }

  value_id_t add(const T& value) override {
    _values.push_back(value);
    return _values.size() - 1;
  }

  value_id_t getSubstitute(const T& value) override {
    auto it = std::lower_bound(ALL(_values), value);
    return std::distance(std::begin(_values), it);
  }

  T getValue(const value_id_t& value_id) const override {
    return _values.at(value_id);
  }

 private:
  std::vector<T> _values;
};

template <typename T>
class UnorderedDictionary final : public BaseDictionary<T> {
 public:
  std::size_t size() const { return _values.size(); }

  value_id_t add(const T& value) {
    auto iter = _index.insert(std::make_pair(value, _values.size()));
    if (iter.second) {
      _values.push_back(value);
    }
    return iter.first->second;
  }

  value_id_t getSubstitute(const T& value) override { return _index[value]; }

  T getValue(const value_id_t& value_id) const override {
    return _values.at(value_id);
  }

 private:
  std::vector<T> _values;
  std::unordered_map<T, value_id_t> _index;
};
