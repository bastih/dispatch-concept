#pragma once

#include <boost/mpl/vector.hpp>
#include <tuple>
#include "storage/dicts.h"
#include "storage/stores.h"
#include "storage/structural.h"

using table_types = boost::mpl::vector<RawTable, Table>;

using storage_types = boost::mpl::vector<FixedStorage, BitStorage<2>, DefaultValueCompressedStorage>;

using dictionary_types = boost::mpl::vector<
  OrderedDictionary<dis_float>, OrderedDictionary<dis_string>,
  UnorderedDictionary<dis_float>, UnorderedDictionary<dis_string>,
  OrderedDictionary<dis_int>, UnorderedDictionary<dis_int> >;

using all_types = boost::mpl::vector<table_types,
                                     storage_types,
                                     dictionary_types>;


using table_types_new = std::tuple<RawTable, Table>;

using storage_types_new = std::tuple<FixedStorage, BitStorage<2>, DefaultValueCompressedStorage>;

using dictionary_types_new = std::tuple<
  OrderedDictionary<dis_float>, OrderedDictionary<dis_string>,
  UnorderedDictionary<dis_float>, UnorderedDictionary<dis_string>,
  OrderedDictionary<dis_int>, UnorderedDictionary<dis_int> >;

using all_types_new = std::tuple<table_types_new,
                                     storage_types_new,
                                     dictionary_types_new>;
