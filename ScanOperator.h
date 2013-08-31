#pragma once

#include <cstdint>
#include "dispatch_fwd.h"

class ATable;

class ScanOperator {
public:
  ScanOperator(ATable* t, std::size_t column, dis_int value);
  void execute();
  void executeFallback();
private:
  ATable* _table;
  std::size_t _column;
  dis_int _value;
};
