#include "storage.h"

type_id_t FixedStorage::typeId = typeid(FixedStorage).hash_code();
type_id_t BitStorage::typeId = typeid(BitStorage).hash_code();
type_id_t Table::typeId = typeid(Table).hash_code();
type_id_t RawTable::typeId = typeid(RawTable).hash_code();

AStorage::~AStorage() {}

FixedStorage::FixedStorage(std::size_t len) : _values(len, 0) {}

void FixedStorage::set(std::size_t x, value_id_t vid) { _values[x] = vid; }

value_id_t FixedStorage::get(std::size_t x) const { return _values[x]; }

const value_id_t& FixedStorage::getRef(std::size_t x) const { return _values[x]; }

BitStorage::BitStorage(std::size_t len) : _values(len, 0) {}

void BitStorage::set(std::size_t x, value_id_t vid) { _values[x] = vid; }

value_id_t BitStorage::get(std::size_t x) const { return _values[x]; }

ATable::~ATable() {}
