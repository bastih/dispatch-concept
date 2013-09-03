#include "storage.h"


AStorage::~AStorage() {}

FixedStorage::FixedStorage(std::size_t len) : _values(len, 0) {}

void FixedStorage::set(std::size_t x, value_id_t vid) { _values[x] = vid; }

value_id_t FixedStorage::get(std::size_t x) const { return _values[x]; }

const value_id_t& FixedStorage::getRef(std::size_t x) const { return _values[x]; }

ATable::~ATable() {}
