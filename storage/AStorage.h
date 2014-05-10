#pragma once

#include "dispatch2/Base.h"
#include "storage/types_fwd.h"

class AStorage : public Base {
 public:
  virtual ~AStorage();
  virtual void set(std::size_t x, value_id_t vid) = 0;
  virtual value_id_t get(std::size_t x) const = 0;
  virtual std::size_t rows() const = 0;
  virtual void structure(std::ostream& out, size_t level=0) const { out << std::string(level, ' ') << "AStorage rows:" << rows() << std::endl; }
};
