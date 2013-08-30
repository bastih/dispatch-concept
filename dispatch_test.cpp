#include "storage.h"
#include "OperatorImpl.h"
#include "debug.hpp"
#include "make_unique.h"
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


std::unique_ptr<ATable> makeMainTable() {
  return make_unique<Table>(make_unique<FixedStorage>(40), make_unique<OrderedDictionary<dis_int>>());
}

std::unique_ptr<ATable> makeDeltaTable() {
  return make_unique<Table>(make_unique<FixedStorage>(20), make_unique<UnorderedDictionary<dis_int>>());
}

std::unique_ptr<ATable> makeStore() {
  std::vector<std::unique_ptr<ATable>> mains, deltas, store_parts;
  for (auto i = 0; i < 3; ++i) {
    mains.emplace_back(makeMainTable());
    deltas.emplace_back(makeDeltaTable());
  }
  store_parts.emplace_back(make_unique<Vertical>(std::move(mains)));
  store_parts.emplace_back(make_unique<Vertical>(std::move(deltas)));
  return make_unique<Horizontal>(std::move(store_parts));
}


int main () {
  auto somestore = makeStore();
  auto parts = somestore->getPartitions(1);
  OperatorImpl<dis_int> o;
  for(auto& part: parts) {
    o.execute(const_cast<ATable*>(part.table),
              const_cast<AStorage*>(part.storage),
              const_cast<ADictionary*>(part.dict));
  }
 
  /*std::cout << store->get(0) << std::endl;
  std::cout << store->get(2) << std::endl;
  std::cout << store->get(1) << std::endl;
  OperatorImpl o;



  o.execute(tab, store, dict); // Executes
  o.execute(tab,store, odict);
  o.execute(tab,store, dict);*/
  return 0;
}

