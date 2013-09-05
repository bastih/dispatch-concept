#pragma once

#include "dispatch/Typed.h"
#include "storage/ADictionary.h"

class ATable;
class ADictionary;
class AStorage;

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
