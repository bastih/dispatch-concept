#pragma once

  // Operator defines special overloads for different combinations of
  // ATable, AStorage, ADictionary descendants, so we can write
  // special implementations

#include "Operator.h"
#include "storage.h"
#include <iostream>

class OperatorImpl : public Operator<OperatorImpl> {
 public:
  /*void execute_special(Table*, FixedStorage* fs, OrderedDictionary*);
    void execute_special(Table*, FixedStorage*, UnorderedDictionary*);*/

  template <typename ValueType>
  void execute_special(Table*, FixedStorage*, UnorderedDictionary<ValueType>* t) {
    std::cout << "Generic implementation based on column type" << std::endl;
    t->size();
  }

  template <typename ValueType>
  void execute_special(Table*, BitStorage<2>*, OrderedDictionary<ValueType>* t) {
    std::cout << "MMM based on column type" << std::endl;
    t->size();
  }

  void execute_special(Table*, BitStorage<2>* bs, OrderedDictionary<dis_int>*);

  void execute_fallback(ATable*, AStorage* as, ADictionary*);
};
