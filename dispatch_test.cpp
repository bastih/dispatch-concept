#include <random>
#include <map>

#include "storage.h"
#include "debug.hpp"
#include "measure.h"
#include "make_unique.h"
#include "ScanOperator.h"
#include "EmptyOperator.h"
#include "FullOperator.h"

constexpr dis_int UPPER_VID = 1000;

std::unique_ptr<FixedStorage> makeFixedStorage(std::size_t sz) {
  auto fs = make_unique<FixedStorage>(sz);
  std::random_device rd;
  std::uniform_int_distribution<int> dist(0, UPPER_VID);
  for (auto i=0ul, e=sz; i < e; ++i) {
    fs->set(i, dist(rd));
  }
  return fs;
}

std::unique_ptr<OrderedDictionary<dis_int> > makeOrderedDict() {
  auto d = make_unique<OrderedDictionary<dis_int> >();
  for (dis_int i=0; i <= UPPER_VID; i++)
    d->add(i);
  return d;
}

std::unique_ptr<UnorderedDictionary<dis_int> > makeUnorderedDict() {
  auto d = make_unique<UnorderedDictionary<dis_int> >();
  for (dis_int i=UPPER_VID; i >= 0; --i)
    d->add(i); // add vids in reverse
  return d;
}

std::unique_ptr<ATable> makeMainTable() {
  return make_unique<Table>(makeFixedStorage(6*1000*1000), makeOrderedDict());
}

std::unique_ptr<ATable> makeDeltaTable() {
  return make_unique<Table>(make_unique<FixedStorage>(1000*1000), makeUnorderedDict());
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
  debug("Generating");
  auto somestore = makeStore();
  somestore->cacheOffsets();
  debug("Done.");

  std::random_device rd;
  std::uniform_int_distribution<int> dist(0, UPPER_VID);
  dis_int value = dist(rd);
  // DICT TESTS
  /*debug(somestore->getValueId(2, 1000*1000-1).vid);
  debug(somestore->getValueId(2, 1000*1000-1).dict);
  debug(somestore->getValueId(2, 1000*1000).vid);
  debug(somestore->getValueId(2, 1000*1000).dict);
  debug(somestore->getValue<dis_int>(2, 1000*1000-1));
  debug(somestore->getValue<dis_int>(2, 1000*1000));*/
  {
    debug("EmptyOperator");
    EmptyOperator so(somestore.get(), 1);
    times_measure("dispatch", [&] () { so.execute(); });
    times_measure("fallback", [&] () { so.executeFallback();  });
  }

  {
    debug("FullOperator");
    FullOperator so(somestore.get(), 1);
    times_measure("dispatch", [&] () { so.execute(); });
    times_measure("fallback", [&] () { so.executeFallback(); });
  }
  

  {
    debug("ScanOperator");
    ScanOperator so(somestore.get(), 1, value);
    times_measure("dispatch", [&] () { so.execute(); });
    times_measure("fallback", [&] () { so.executeFallback(); });
    times_measure("abstract", [&] () { so.executeAbstract(); });
    times_measure("perfect ", [&] () { so.executePerfect(); });
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

