#include "storage.h"
#include "OperatorImpl.h"
#include "debug.hpp"

int main () {
  ADictionary* dict = new UnorderedDictionary<dis_int>;
  ADictionary* odict = new OrderedDictionary<dis_int>;
  ATable* tab = new Table;
  AStorage* store = new FixedStorage(10);
  AStorage* bitstore = new BitStorage(10);
  OperatorImpl o;

  o.execute(tab, store, dict); // Executes
  debug("TAB, BS, OD");
  o.execute(tab, bitstore, odict);
  debug("No specialization:");
  o.execute(tab, bitstore, dict);
  return 0;
}
