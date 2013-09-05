#pragma once

#include <bitset>

#include "storage/AStorage.h"
#include "storage/types_fwd.h"
#include "helpers/shortcuts.h"

#include <boost/container/flat_map.hpp>
#include <boost/dynamic_bitset.hpp>

class DefaultValueCompressedStorage : public AStorage {
 public:
  explicit DefaultValueCompressedStorage(std::size_t len, value_id_t value)
      : _default_value(value), _exception_positions(len, true) {}

  void createPositionList(value_id_t vid, std::size_t offset, std::vector<std::size_t>& positions) const;
  void createPositionListFromDefault(std::size_t offset, std::vector<std::size_t>& positions) const;
  void createPositionListForException(value_id_t vid, std::size_t offset, std::vector<std::size_t>& positions) const;

  void set(std::size_t x, value_id_t vid) override;
  value_id_t get(std::size_t x) const override;
  std::size_t rows() const override;

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

template <int N>
class BitStorage final : public AStorage {
  static constexpr int values_per_interval = 64 / N;
 public:
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
