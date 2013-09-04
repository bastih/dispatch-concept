#pragma once
#include <algorithm>
#include <vector>
#include <cstdint>
#include <unordered_map>
#include <stdexcept>
#include <vector>
#include <memory>

#include <boost/container/flat_map.hpp>
#include <boost/dynamic_bitset.hpp>

#include "dispatch.h"
#include "storage_types.h"
#include "helpers/Range.h"

#define ALL(var) std::begin(var), std::end(var)
#define _AUTO(var) decltype(*std::begin(var))
#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

using value_id_t = std::uint32_t;

namespace std {
inline std::string to_string(const std::string& s) { return s; }
}

class ADictionary : public Typed {
 public:
  ~ADictionary();
  virtual std::string getValueString(value_id_t) const = 0;
};

template <typename T>
class BaseDictionary : public ADictionary {
 public:
  virtual std::size_t size() const = 0;
  virtual value_id_t add(const T&) = 0;
  virtual value_id_t getSubstitute(const T&) = 0;
  virtual T getValue(const value_id_t&) const = 0;
  std::string getValueString(const value_id_t vid) const override {
    return std::to_string(getValue(vid));
  }

};

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

class AStorage : public Typed {
 public:
  ~AStorage();
  virtual void set(std::size_t x, value_id_t vid) = 0;
  virtual value_id_t get(std::size_t x) const = 0;
  virtual std::size_t rows() const = 0;
};

class DefaultValueCompressedStorage : public AStorage {
 public:
  ~DefaultValueCompressedStorage() {}
  explicit DefaultValueCompressedStorage(std::size_t len, value_id_t value)
      : _default_value(value), _exception_positions(len, true) {}

  void set(std::size_t x, value_id_t vid) {
    if (likely(vid == _default_value)) {
      if (likely(_exception_positions[x] == false)) {
        return;
      } else {
        _exceptions.erase(x);
        _exception_positions[x] = false;
      }
    } else {
      _exception_positions[x] = true;
      _exceptions[x] = vid;
    }
  }

  value_id_t get(std::size_t x) const {
    if (unlikely(_exception_positions[x]))
      return _exceptions.at(x);
    else
      return _default_value;
  }

  void createPositionList(value_id_t vid, std::size_t offset,
                          std::vector<std::size_t>& positions) {
    if (likely(vid == _default_value))
      createPositionListFromDefault(offset, positions);
    else
      createPositionListForException(vid, offset, positions);
  }

  void createPositionListFromDefault(
      std::size_t offset, std::vector<std::size_t>& positions) const {
    std::size_t start = _exception_positions.find_first();
    std::size_t set_bit_pos;
    std::vector<Range> ranges;
    std::size_t sum_new_bits = 0;
    while ((set_bit_pos = _exception_positions.find_next(start)) !=
           boost::dynamic_bitset<>::npos) {
      std::size_t num_new_bits = set_bit_pos - start;
      sum_new_bits += num_new_bits;
      ranges.emplace_back(start, set_bit_pos);
      start = set_bit_pos;
    }
    positions.resize(positions.size() + sum_new_bits);
    for (const auto& range : ranges) {
      std::iota(positions.begin() + offset + range._start,
                positions.begin() + offset + range._stop,
                range._start + offset);
    }
  }
  void createPositionListForException(
      value_id_t vid, std::size_t offset,
      std::vector<std::size_t>& positions) const {
    for (const auto& kv : _exceptions) {
      if (kv.second == vid) {
        positions.push_back(offset + kv.first);
      }
    }
  }
  std::size_t rows() const { return _exception_positions.size(); }

 private:
  const value_id_t _default_value;
  boost::dynamic_bitset<> _exception_positions;
  boost::container::flat_map<std::size_t, value_id_t> _exceptions;
};

class FixedStorage final : public AStorage {
 public:
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

template <int N>
class BitStorage final : public AStorage {
 public:
  static constexpr int values_per_interval = 64 / N;
  explicit BitStorage(std::size_t len) { _values.resize(N * len / 64 + 1); }

  void set(std::size_t x, value_id_t vid) {
    assert(vid < (1l << N) && "vid must be smaller than 2^n");
    auto interval = x / values_per_interval;
    auto offset_in_value = (x % values_per_interval) * N;
    auto& bs = *(std::bitset<64>*)(_values.data() + interval);
    auto& value_bs = *(std::bitset<N>*)&vid;
    for (std::size_t i = 0; i < N; ++i) {
      bs[offset_in_value + i] = value_bs[i];
    }
  }

  value_id_t get(std::size_t x) const {
    auto interval = x / values_per_interval;
    auto offset_in_value = (x % values_per_interval) * N;
    auto bs = *(std::bitset<64>*)&_values[interval];
    std::bitset<N> result;
    for (std::size_t i = 0; i < N; ++i) {
      result[i] = bs[offset_in_value + i];
    }
    return result.to_ulong();
  }

  std::size_t rows() const override {
    return _values.size() * values_per_interval;
  }

 private:
  std::vector<std::uint64_t> _values;
};

typedef struct {
  std::size_t start;
  std::size_t stop;
  std::size_t offset;
  const ATable* table;
  const AStorage* storage;
  const ADictionary* dict;
} partition_t;

typedef struct {
  value_id_t vid;
  const ADictionary* dict;
} value_id_with_dict_t;

typedef std::vector<partition_t> partitions_t;

class ATable : public Typed {
 public:
  ~ATable();
  virtual std::size_t width() const = 0;
  virtual std::size_t height() const = 0;
  virtual partitions_t getVerticalPartitions(std::size_t column) const = 0;
  virtual partitions_t getHorizontalPartitions(std::size_t row) const = 0;
  /// BAAAAD GURL
  template <typename T>
  T getValue(std::size_t col, std::size_t row) const {
    auto val_dct = getValueId(col, row);
    return static_cast<const BaseDictionary<T>*>(val_dct.dict)
        ->getValue(val_dct.vid);
  }
  virtual void cacheOffsets() {}
  virtual value_id_with_dict_t getValueId(std::size_t col,
                                          std::size_t row) const = 0;
};

/* HYBRID DATABASE */
class Vertical : public ATable {
 public:
  Vertical(std::vector<std::unique_ptr<ATable> > parts)
      : _parts(std::move(parts)) {}
  std::size_t width() const override;
  std::size_t height() const override { return _parts.front()->height(); }
  partitions_t getVerticalPartitions(std::size_t column) const override;
  partitions_t getHorizontalPartitions(std::size_t row) const override;

  /// BAD GURRRRRL
  void cacheOffsets() override {
    for (const auto col : Range(width())) {
      _cached_offsets.push_back(partForColumn(col));
    }
    for (auto& part : _parts) {
      part->cacheOffsets();
    }
  }
  std::vector<std::pair<std::size_t, std::size_t> > _cached_offsets;
  value_id_with_dict_t getValueId(std::size_t col, std::size_t row) const {
    auto p = _cached_offsets[col];
    return _parts[p.first]->getValueId(p.second, row);
  }

 private:
  std::pair<std::size_t, std::size_t> partForColumn(std::size_t column) const;
  std::vector<std::unique_ptr<ATable> > _parts;
};

class Horizontal : public ATable {
 public:
  Horizontal(std::vector<std::unique_ptr<ATable> > parts)
      : _parts(std::move(parts)) {}
  std::size_t width() const override;
  std::size_t height() const override {
    return std::accumulate(ALL(_parts),
                           0u, [](std::size_t r, _AUTO(_parts) part) {
      return r + part->height();
    });
  }
  partitions_t getVerticalPartitions(std::size_t column) const;
  partitions_t getHorizontalPartitions(std::size_t row) const;

  void cacheOffsets() override {
    std::size_t offset = 0;
    for (const auto& part : _parts) {
      offset += part->height();
      _cached_offsets.push_back(offset);
      part->cacheOffsets();
    }
  }
  std::vector<std::size_t> _cached_offsets;

  /// BAD GURRLLL
  value_id_with_dict_t getValueId(std::size_t col, std::size_t row) const
      override {
    std::size_t part = 0;
    std::size_t offset = 0;
    for (std::size_t coffset : _cached_offsets) {
      if (offset + coffset > row) break;
      part++;
      offset += coffset;
    }
    return _parts[part]->getValueId(col, row - offset);
  }

 private:
  std::pair<std::size_t, std::size_t> partForRow(std::size_t row) const {
    std::size_t offset = 0, part_index = 0;
    for (const auto& part : _parts) {
      if (offset + part->height() > row) {
        return std::make_pair(part_index, row - offset);
      }
      offset += part->height();
      part_index++;
    }
    throw std::runtime_error("sucks");
  }
  std::vector<std::unique_ptr<ATable> > _parts;
};

class Table final : public ATable {
 public:
  Table(std::unique_ptr<AStorage> s, std::unique_ptr<ADictionary> d)
      : _storage(std::move(s)), _dictionary(std::move(d)) {}
  std::size_t width() const override;
  std::size_t height() const override { return _storage->rows(); }
  partitions_t getVerticalPartitions(std::size_t column) const override;
  partitions_t getHorizontalPartitions(std::size_t row) const override;
  value_id_with_dict_t getValueId(std::size_t col, std::size_t row) const
      override {
    assert(col == 0);
    // usually we would have to pass col too, but we currently assume
    // single-width tables
    return {_storage->get(row), _dictionary.get()};
  }

 private:
  std::unique_ptr<AStorage> _storage;
  std::unique_ptr<ADictionary> _dictionary;
};

class RawTable final : public ATable {
 public:
 private:
};
