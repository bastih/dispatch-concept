#pragma once

#include <type_traits>
#include <typeindex>

using type_id_t = std::size_t;

template <typename T>
inline static std::size_t typeId() {
  static type_id_t id = typeid(T).hash_code();
  return id;
}

class Typed {
 public:
  virtual ~Typed();
  type_id_t getTypeId() const { return typeid(*this).hash_code(); }
};
