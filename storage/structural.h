#pragma once

#include <cstdint>
#include <vector>
#include <memory>

#include "storage/ATable.h"
#include "storage/AStorage.h"

/* HYBRID DATABASE */
class Vertical : public ATable {
 public:
  Vertical(std::vector<std::unique_ptr<ATable> > parts)
      : _parts(std::move(parts)) {}
  std::size_t width() const override;
  std::size_t height() const override;
  partitions_t getVerticalPartitions(std::size_t column) const override;
  partitions_t getHorizontalPartitions(std::size_t row) const override;

  /// BAD GURRRRRL
  void cacheOffsets() override;
  value_id_with_dict_t getValueId(std::size_t col, std::size_t row) const;
 private:
  std::pair<std::size_t, std::size_t> partForColumn(std::size_t column) const;
  std::vector<std::unique_ptr<ATable> > _parts;
  std::vector<std::pair<std::size_t, std::size_t> > _cached_offsets;
};

class Horizontal : public ATable {
 public:
  Horizontal(std::vector<std::unique_ptr<ATable> > parts)
      : _parts(std::move(parts)) {}
  std::size_t width() const override;
  std::size_t height() const override;
  partitions_t getVerticalPartitions(std::size_t column) const override;
  partitions_t getHorizontalPartitions(std::size_t row) const override;

  void cacheOffsets() override;
  value_id_with_dict_t getValueId(std::size_t col, std::size_t row) const override;
 private:
  std::pair<std::size_t, std::size_t> partForRow(std::size_t row) const;
  std::vector<std::size_t> _cached_offsets;
  std::vector<std::unique_ptr<ATable> > _parts;
};

class Table final : public ATable {
 public:
  Table(std::unique_ptr<AStorage> s, std::unique_ptr<ADictionary> d)
      : _storage(std::move(s)), _dictionary(std::move(d)) {}
  std::size_t width() const override;
  std::size_t height() const override;
  partitions_t getVerticalPartitions(std::size_t column) const override;
  partitions_t getHorizontalPartitions(std::size_t row) const override;
  value_id_with_dict_t getValueId(std::size_t col, std::size_t row) const
      override;
 private:
  std::unique_ptr<AStorage> _storage;
  std::unique_ptr<ADictionary> _dictionary;
};

class RawTable final : public ATable {
 public:
 private:
};
