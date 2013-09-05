#pragma once

#include "dispatch/Typed.h"
#include "storage/types_fwd.h"

class AStorage : public Typed {
 public:
  virtual ~AStorage();
  virtual void set(std::size_t x, value_id_t vid) = 0;
  virtual value_id_t get(std::size_t x) const = 0;
  virtual std::size_t rows() const = 0;
};