#include "structural.h"

#include <cassert>
#include <algorithm>

#include "helpers/shortcuts.h"
#include "helpers/Range.h"

ATable::~ATable() {}



std::size_t Vertical::width() const {
  return std::accumulate(ALL(_parts), 0u, [](std::size_t r, _AUTO(_parts) part) {
    return r + part->width();
  });
}

std::size_t Vertical::height() const {
  return _parts.front()->height();
}

partitions_t Vertical::getVerticalPartitions(std::size_t column) const {
  std::size_t part_index, column_offset;
  std::tie(part_index, column_offset) = partForColumn(column);
  return _parts[part_index]->getVerticalPartitions(column_offset);
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

void Vertical::cacheOffsets() {
  for (const auto col : Range(width())) {
    _cached_offsets.push_back(partForColumn(col));
  }
  for (auto& part : _parts) {
    part->cacheOffsets();
  }
}

value_id_with_dict_t Vertical::getValueId(std::size_t col, std::size_t row) const {
  auto p = _cached_offsets[col];
  return _parts[p.first]->getValueId(p.second, row);
}



std::size_t Horizontal::width() const { return _parts.at(0)->width(); }
std::size_t Horizontal::height() const {
  return std::accumulate(ALL(_parts),
                         0u, [](std::size_t r, _AUTO(_parts) part) {
                           return r + part->height();
                         });
}

partitions_t Horizontal::getHorizontalPartitions(std::size_t row) const {
  std::size_t part_index, row_offset;
  std::tie(part_index, row_offset) = partForRow(row);
  return _parts[part_index]->getHorizontalPartitions(row_offset);
}

partitions_t Horizontal::getVerticalPartitions(std::size_t column) const {
  partitions_t r;
  std::size_t height_offset = 0u;
  for (const auto& part : _parts) {
    auto p = part->getVerticalPartitions(column);
    for (partition_t& subpart : p) {
      subpart.start = height_offset;
      height_offset += subpart.stop;
      subpart.stop = height_offset;
    }
    r.insert(std::begin(r), ALL(p));
  }
  return r;
}

void Horizontal::cacheOffsets() {
  std::size_t offset = 0;
  for (const auto& part : _parts) {
    offset += part->height();
    _cached_offsets.push_back(offset);
    part->cacheOffsets();
  }
}


value_id_with_dict_t Horizontal::getValueId(std::size_t col, std::size_t row) const {
  std::size_t part = 0;
  std::size_t offset = 0;
  for (std::size_t coffset : _cached_offsets) {
    if (offset + coffset > row) break;
    part++;
    offset += coffset;
  }
  return _parts[part]->getValueId(col, row - offset);
}

std::pair<std::size_t, std::size_t> Horizontal::partForRow(std::size_t row) const {
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

std::size_t Table::height() const { return _storage->rows(); }

std::size_t Table::width() const { return 1; }
partitions_t Table::getVerticalPartitions(std::size_t column) const {
  assert(column < width());
  return {partition_t{0, _storage->rows(), column, this, _storage.get(), _dictionary.get()}};
}

partitions_t Table::getHorizontalPartitions(std::size_t row) const {
  assert(row < height());
  return {partition_t{0, 1, row, this, _storage.get(), _dictionary.get()}};
}

value_id_with_dict_t Table::getValueId(std::size_t col, std::size_t row) const {
  assert(col == 0);
  // usually we would have to pass col too, but we currently assume
  // single-width tables
  return {_storage->get(row), _dictionary.get()};
}
