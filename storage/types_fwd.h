#pragma once

#include <cstdint>
#include <string>

using dis_int = std::int32_t;
using dis_float = float;
using dis_string = std::string;
using value_id_t = std::uint32_t;

class Typed;
class ATable;
class ADictionary;
class AStorage;

class Table;
class RawTable;

class FixedStorage;
class DefaultValueCompressedStorage;
template <int> class BitStorage;

template <typename T> class OrderedDictionary;
template <typename T> class UnorderedDictionary;
