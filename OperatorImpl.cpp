#include "OperatorImpl.h"

#include "debug.hpp"

void OperatorImpl::execute_special(Table*, BitStorage<2>* bs, OrderedDictionary<dis_int>*) {
    debug("Special for Table*, BitStorage*, OrderedDictionary*");
    bs->get(1); // Not a virtual function call
}

void OperatorImpl::execute_fallback(ATable*, AStorage*, ADictionary*) {
  debug("Fallback kicks in");
  //as->get(0);
}
