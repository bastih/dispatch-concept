#pragma once

#include <boost/mpl/vector.hpp>
#include <cstdint>

using dis_int = std::int32_t;
using dis_float = float;
using dis_string = std::string;

class ATable;
class ADictionary;
class AStorage;

class Table;
class RawTable;
using table_types = boost::mpl::vector<Table, RawTable>;

class FixedStorage;
class BitStorage;
using storage_types = boost::mpl::vector<FixedStorage, BitStorage>;

template <typename T> class OrderedDictionary;
template <typename T> class UnorderedDictionary;

using dictionary_types = boost::mpl::vector<
  OrderedDictionary<dis_int>,
  OrderedDictionary<dis_float>,
  OrderedDictionary<dis_string>,
  UnorderedDictionary<dis_int>,
  UnorderedDictionary<dis_float>,
  UnorderedDictionary<dis_string> >;
