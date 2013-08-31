#pragma once
#include <algorithm>
#include <vector>
#include <cstdint>
#include <unordered_map>
#include <stdexcept>
#include <vector>
#include <memory>

#include "dispatch.h"
#include "storage_types.h"
#include "like_const.h"

#define ALL(var) std::begin(var), std::end(var)
#define _AUTO(var) decltype(*std::begin(var))

using value_id_t = std::uint64_t;

class ADictionary : public Typed {
 public:

};

template <typename T>
class BaseDictionary : public ADictionary {
 public:
  virtual std::size_t size() const = 0;
  virtual value_id_t add(const T&) = 0;
  virtual value_id_t getSubstitute(const T&) = 0;
  virtual T get(const value_id_t&) const = 0;
};

template <typename T>
class OrderedDictionary final : public BaseDictionary<T> {
public:
  static type_id_t typeId;

  std::size_t size() const { return _values.size(); }

  value_id_t add(const T& value) override {
    _values.push_back(value);
    return _values.size() - 1;
  }

  value_id_t getSubstitute(const T& value) override {
    auto it = std::lower_bound(ALL(_values), value);
    return std::distance(std::begin(_values), it);
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

  value_id_t getSubstitute(const T& value) override {
    return _index[value];
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
  virtual std::size_t rows() const = 0;
};

class FixedStorage final : public AStorage {
 public:
  static  type_id_t typeId;
  explicit FixedStorage(std::size_t len);
  void set(std::size_t x, value_id_t vid);
  value_id_t get(std::size_t x) const;
  const value_id_t& getRef(std::size_t x) const;
  std::size_t rows() const { return _values.size(); }
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

  std::size_t rows() const override {
    return _values.size() * values_per_interval;
  }
private:
  std::vector<std::uint64_t> _values;
};

template <int N>
type_id_t BitStorage<N>::typeId = typeid(BitStorage<N>).hash_code();

typedef struct {
  std::size_t start;
  std::size_t stop;
  std::size_t offset;
  const ATable* table;
  const AStorage* storage;
  const ADictionary* dict;
} partition_t;

typedef std::vector<partition_t> partitions_t;

class ATable : public Typed {
 public:
  ~ATable();
  virtual std::size_t width() const = 0;
  virtual partitions_t getPartitions(std::size_t column) const = 0;
};


/* HYBRID DATABASE */
class Vertical : public ATable {
 public:
  Vertical(std::vector<std::unique_ptr<ATable>> parts) : _parts(std::move(parts)) {}
  std::size_t width() const override {
    return std::accumulate(ALL(_parts), 0u, [] (std::size_t r, _AUTO(_parts) part) { return r + part->width(); });
  }
  partitions_t getPartitions(std::size_t column) const override {
    std::size_t part_index, column_offset;
    std::tie(part_index, column_offset) = partForColumn(column);
    return _parts[part_index]->getPartitions(column_offset);
  }

 private:
  std::pair<std::size_t, std::size_t> partForColumn(std::size_t column) const {
    std::size_t column_offset = 0u;
    std::size_t part_index = 0u;
    for (const auto& part : _parts) {
      if (column_offset + part->width() > column) {
        return std::make_pair(part_index, column - column_offset);
      }
      part_index++;
      column_offset += part->width();
    }
    throw std::runtime_error("Could not find column");
  }
  std::vector<std::unique_ptr<ATable>> _parts;
};

class Horizontal : public ATable {
 public:
  Horizontal(std::vector<std::unique_ptr<ATable>> parts) : _parts(std::move(parts)) {}
  std::size_t width() const override {
    return _parts.at(0)->width();
  }

  partitions_t getPartitions(std::size_t column) const override {
    partitions_t r;
    std::size_t height_offset = 0u;
    for (const auto& part: _parts) {
      auto p = part->getPartitions(column);
      for (partition_t& subpart: p) {
        subpart.start = height_offset;
        height_offset += subpart.stop;
        subpart.stop = height_offset;
      }
      r.insert(std::begin(r), ALL(p));
    }
    return r;
  }

 private:
  std::vector<std::unique_ptr<ATable>> _parts;
};

class Table final : public ATable {
public:
  static type_id_t typeId;
  Table() = default;
  Table(std::unique_ptr<AStorage> s, std::unique_ptr<ADictionary> d) : _storage(std::move(s)), _dictionary(std::move(d)) {}
  std::size_t width() const override {
    return 1;
  }
  partitions_t getPartitions(std::size_t column) const override {
    assert(column < width());
    return { partition_t { 0, _storage->rows(), column, this, _storage.get(), _dictionary.get() }};
  }
private:
  std::unique_ptr<AStorage> _storage;
  std::unique_ptr<ADictionary> _dictionary;
};

class RawTable final : public ATable {
public:
  static  type_id_t typeId;
private:
};
