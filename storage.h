#pragma once

#include <vector>
#include <cstdint>
#include <unordered_map>
#include <stdexcept>

#include "dispatch.h"
#include "storage_types.h"

using value_id_t = std::uint64_t;

class ADictionary : public Typed {
 public:

};

template <typename T>
class BaseDictionary : public ADictionary {
 public:
  virtual std::size_t size() const = 0;
  virtual value_id_t add(const T&) = 0;
  virtual T get(const value_id_t&) const = 0;
};

template <typename T>
class OrderedDictionary final : public BaseDictionary<T> {
public:
  static type_id_t typeId;

  std::size_t size() const { return _values.size(); }

  value_id_t add(const T& value) override {
    if (value <= _values.back()) {
      throw std::runtime_error("may only insert linearly");
    }
    _values.push_back(value);
    return _values.size() - 1;
  }

  T get(const value_id_t& value_id) const override {
    return _values.at(value_id);
  }


private:
  std::vector<T> _values;
};

template <typename T>
type_id_t OrderedDictionary<T>::typeId = typeid(OrderedDictionary<T>).hash_code();

template <typename T>
class UnorderedDictionary final : public BaseDictionary<T> {
public:
  static type_id_t typeId;

  std::size_t size() const { return _values.size(); }

  value_id_t add(const T& value) {
    auto iter = _index.insert(std::make_pair(value, _values.size() + 1));
    if (iter.second) {
      _values.push_back(value);
    }
    return iter.first->second;
  }

  T get(const value_id_t& value_id) const override {
    return _values.at(value_id);
  }
private:
  std::vector<T> _values;
  std::unordered_map<T, value_id_t> _index;
};

template <typename T>
type_id_t UnorderedDictionary<T>::typeId = typeid(UnorderedDictionary<T>).hash_code();

class AStorage : public Typed {
 public:
  ~AStorage();
  virtual void set(std::size_t x, value_id_t vid) = 0;
  virtual value_id_t get(std::size_t x) const = 0;
};

class FixedStorage final : public AStorage {
 public:
  static  type_id_t typeId;
  explicit FixedStorage(std::size_t len);
  void set(std::size_t x, value_id_t vid);
  value_id_t get(std::size_t x) const;
  const value_id_t& getRef(std::size_t x) const;
 private:
  std::vector<value_id_t> _values;
};

#include <bitset>
#include <cassert>
#include "debug.hpp"

template <int N>
class BitStorage final : public AStorage {
public:
  static type_id_t typeId;
  static constexpr int values_per_interval = 64/N;
  explicit BitStorage(std::size_t len) { _values.resize(N*len/64+1); }

  void set(std::size_t x, value_id_t vid) {
    assert(vid < (1l << N) && "vid must be smaller than 2^n");
    auto interval = x / values_per_interval;
    auto offset_in_value = (x % values_per_interval) * N;
    auto& bs = *(std::bitset<64>*) (_values.data() + interval);
    auto& value_bs = *(std::bitset<N>*) &vid;
    for (std::size_t i = 0; i < N; ++i) {
      bs[offset_in_value+i] = value_bs[i];
    }
  }

  value_id_t get(std::size_t x) const {
    auto interval = x / values_per_interval;
    auto offset_in_value = (x % values_per_interval) * N;
    auto bs = *(std::bitset<64>*) &_values[interval];
    std::bitset<N> result;
    for (std::size_t i = 0; i < N; ++i) {
      result[i] = bs[offset_in_value+i];
    }
    return result.to_ulong();
  }

private:
  std::vector<std::uint64_t> _values;
};












template <int N>
type_id_t BitStorage<N>::typeId = typeid(BitStorage<N>).hash_code();


class ATable : public Typed {
 public:
  ~ATable();
};

class Table final : public ATable {
public:
  static  type_id_t typeId;
};



class RawTable final : public ATable {
  static  type_id_t typeId;
};
