#pragma once

#include "dispatch2/Base.h"
#include "storage/types_fwd.h"

namespace std {
inline std::string to_string(const std::string& s) { return s; }
}

class ADictionary : public Base {
 public:
  ~ADictionary();
  virtual std::string getValueString(value_id_t) const = 0;
};

template <typename T>
class BaseDictionary : public ADictionary {
 public:
  virtual std::size_t size() const = 0;
  virtual value_id_t add(const T&) = 0;
  virtual value_id_t getSubstitute(const T&) = 0;
  virtual T getValue(const value_id_t&) const = 0;
  std::string getValueString(const value_id_t vid) const override {
    return std::to_string(getValue(vid));
  }
};
