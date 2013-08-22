#include "storage.h"
#include "OperatorImpl.h"
#include "debug.hpp"

#include <random>

AStorage* randStore() {
  std::random_device rd;
  std::uniform_int_distribution<int> dist(0, 1);
  if (dist(rd)) {
    return new FixedStorage(100);
  } else {
    return new BitStorage<2>(100);
  }
}

int main () {
  ADictionary* dict = new UnorderedDictionary<dis_int>;
  ADictionary* odict = new OrderedDictionary<dis_int>;
  ATable* tab = new Table;
AStorage* store = randStore();

  store->set(0, 0);
  store->set(1, 1);
  store->set(2, 2);
  store->set(4, 3);
  store->set(64, 1);
  store->set(65, 1);
  store->set(99, 3);
store->set(97, 3);
store->set(98, 3);

  std::cout << store->get(0) << std::endl;
  std::cout << store->get(2) << std::endl;
  std::cout << store->get(1) << std::endl;
  OperatorImpl o;

  o.execute(tab, store, dict); // Executes
  o.execute(tab,store, odict);
  o.execute(tab,store, dict);
  return 0;
}
