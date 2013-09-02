#pragma once

#include <cstdint>

class ATable;

class EmptyOperator {
 public:
  EmptyOperator(ATable* table, std::size_t column);
  void execute();
  void executeFallback();
 private:
  ATable* _table;
  std::size_t _column;
};
