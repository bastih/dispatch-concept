#pragma once

#include <type_traits>
#include <typeindex>

using type_id_t = std::size_t;

class Typed {
 public:
  virtual ~Typed();
  virtual type_id_t getTypeId() const { return typeid(*this).hash_code(); }
};
