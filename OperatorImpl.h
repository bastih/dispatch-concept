#pragma once

  // Operator defines special overloads for different combinations of
  // ATable, AStorage, ADictionary descendants, so we can write
  // special implementations

#include "Operator.h"
#include "storage.h"
#include <iostream>

template <typename T>
class OperatorImpl : public Operator<OperatorImpl<T>> {
 public:
  template <typename ValueType=T>
  void execute_special(Table*, FixedStorage*, OrderedDictionary<ValueType>* t) {
    debug("Specialization Table, FixedStorage,", typeid(t).name());
  }

  template <typename ValueType=T>
  void execute_special(Table*, FixedStorage* bs, UnorderedDictionary<ValueType>* t) {
    debug("Specialization Table, FixedStorage,", typeid(t).name());
  };

  void execute_fallback(ATable* t, AStorage* s, ADictionary* d) {
    debug("Fallback kicks in, types are");
    debug(typeid(t).name(), "\n", typeid(s).name(), "\n", typeid(d).name());
  }
};
