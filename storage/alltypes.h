#pragma once

#include <tuple>
#include "storage/dicts.h"
#include "storage/stores.h"
#include "storage/structural.h"

using table_types = std::tuple<Table*>;

using storage_types = std::tuple<FixedStorage*, BitStorage<2>*, DefaultValueCompressedStorage*>;

using dictionary_types = std::tuple<
  OrderedDictionary<dis_float>*, OrderedDictionary<dis_string>*,
  UnorderedDictionary<dis_float>*, UnorderedDictionary<dis_string>*,
  OrderedDictionary<dis_int>*, UnorderedDictionary<dis_int>* 
>;
