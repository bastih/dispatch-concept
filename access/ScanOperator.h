#pragma once

#include <cstdint>

#include "storage/types_fwd.h"

class ScanOperator {
 public:
  ScanOperator(ATable* t, std::size_t column, dis_int value);
  void execute();
  void executeFallback();
  void executeAbstract();
  void executePerfect();

 private:
  ATable* _table;
  std::size_t _column;
  dis_int _value;
};
