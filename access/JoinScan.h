#pragma once

#include "storage/types_fwd.h"

class JoinScan {
 public:
  JoinScan(ATable* outer, ATable* inner, col_t outer_col, col_t inner_col) :
      _outer(outer), _inner(inner), _outer_col(outer_col), _inner_col(inner_col) {}
  void execute();
  void executeFallback();
  void executeAbstract();
 private:
  ATable* _outer, *_inner, *_result;
  col_t _outer_col, _inner_col;
};
