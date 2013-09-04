#include "storage.h"

ADictionary::~ADictionary() = default;

AStorage::~AStorage() = default;

FixedStorage::FixedStorage(std::size_t len) : _values(len, 0) {}

void FixedStorage::set(std::size_t x, value_id_t vid) { _values[x] = vid; }

value_id_t FixedStorage::get(std::size_t x) const { return _values[x]; }

const value_id_t& FixedStorage::getRef(std::size_t x) const { return _values[x]; }

ATable::~ATable() {}

std::size_t Vertical::width() const {
  return std::accumulate(ALL(_parts), 0u, [](std::size_t r, _AUTO(_parts) part) {
    return r + part->width();
  });
}

partitions_t Vertical::getPartitions(std::size_t column) const {
  std::size_t part_index, column_offset;
  std::tie(part_index, column_offset) = partForColumn(column);
  return _parts[part_index]->getPartitions(column_offset);
}

partitions_t Vertical::getHorizontalPartitions(std::size_t row) const {
  partitions_t r;
  std::size_t col_offset = 0u;
  for (const auto& part : _parts) {
    auto p = part->getHorizontalPartitions(row);
    for (partition_t& subpart : p) {
      subpart.start = col_offset;
      col_offset += subpart.stop;
      subpart.stop = col_offset;
    }
    r.insert(std::begin(r), ALL(p));
  }
  return r;
}

std::pair<std::size_t, std::size_t> Vertical::partForColumn(std::size_t column) const {
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

std::size_t Horizontal::width() const { return _parts.at(0)->width(); }

partitions_t Horizontal::getHorizontalPartitions(std::size_t row) const {
  std::size_t part_index, row_offset;
  std::tie(part_index, row_offset) = partForRow(row);
  return _parts[part_index]->getHorizontalPartitions(row_offset);
}

partitions_t Horizontal::getPartitions(std::size_t column) const {
  partitions_t r;
  std::size_t height_offset = 0u;
  for (const auto& part : _parts) {
    auto p = part->getPartitions(column);
    for (partition_t& subpart : p) {
      subpart.start = height_offset;
      height_offset += subpart.stop;
      subpart.stop = height_offset;
    }
    r.insert(std::begin(r), ALL(p));
  }
  return r;
}

std::size_t Table::width() const { return 1; }
partitions_t Table::getPartitions(std::size_t column) const {
  assert(column < width());
  return {partition_t{0, _storage->rows(), column, this, _storage.get(), _dictionary.get()}};
}

partitions_t Table::getHorizontalPartitions(std::size_t row) const {
  assert(row < height());
  return {partition_t{0, 1, row, this, _storage.get(), _dictionary.get()}};
}
