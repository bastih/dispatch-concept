#include "storage/stores.h"

#include "helpers/Range.h"

FixedStorage::FixedStorage(std::size_t len) : _values(len, 0) {}

void FixedStorage::set(std::size_t x, value_id_t vid) { _values[x] = vid; }

value_id_t FixedStorage::get(std::size_t x) const {
  return _values[x];
}

const value_id_t& FixedStorage::getRef(std::size_t x) const { return _values[x]; }

void DefaultValueCompressedStorage::set(std::size_t x, value_id_t vid) {
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

value_id_t DefaultValueCompressedStorage::get(std::size_t x) const {
  if (unlikely(_exception_positions[x]))
    return _exceptions.at(x);
  else
    return _default_value;
}

void DefaultValueCompressedStorage::createPositionList(value_id_t vid, std::size_t offset,
                                                       std::vector<std::size_t>& positions) const {
  if (likely(vid == _default_value))
    createPositionListFromDefault(offset, positions);
  else
    createPositionListForException(vid, offset, positions);
}

void DefaultValueCompressedStorage::createPositionListFromDefault(
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
void DefaultValueCompressedStorage::createPositionListForException(
    value_id_t vid, std::size_t offset,
    std::vector<std::size_t>& positions) const {
  for (const auto& kv : _exceptions) {
    if (kv.second == vid) {
      positions.push_back(offset + kv.first);
    }
  }
}
std::size_t DefaultValueCompressedStorage::rows() const { return _exception_positions.size(); }
