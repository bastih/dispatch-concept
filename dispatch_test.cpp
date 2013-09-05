#include "helpers/debug.hpp"
#include "helpers/measure.h"
#include "Generator.h"

#include "ScanOperator.h"
#include "EmptyOperator.h"
#include "FullOperator.h"
#include "MaterializingScan.h"

#include "storage/structural.h"

int main(int argc, char* const argv[]) {
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
    debug("MaterializingScanOperator");
    MaterializingScanOperator so(somestore.get(), 3);
    times_measure("dispatch", [&so]() {
      so.execute();
    });
    times_measure("fallback", [&so]() {
      so.executeFallback();
    });
    times_measure("abstract", [&so]() {
      so.executeAbstract();
    });
  }

  {
    debug("EmptyOperator");
    EmptyOperator so(somestore.get(), 1);
    times_measure("dispatch", [&]() {
      so.execute();
    });
    times_measure("fallback", [&]() {
      so.executeFallback();
    });
  }

  {
    debug("FullOperator");
    FullOperator so(somestore.get(), 1);
    times_measure("dispatch", [&]() {
      so.execute();
    });
    times_measure("fallback", [&]() {
      so.executeFallback();
    });
  }

  {
    debug("ScanOperator on FixedLengthStorage");
    ScanOperator so(somestore.get(), 1, value);
    times_measure("dispatch", [&]() {
      so.execute();
    });
    times_measure("fallback", [&]() {
      so.executeFallback();
    });
    times_measure("abstract", [&]() {
      so.executeAbstract();
    });
    times_measure("perfect ", [&]() {
      so.executePerfect();
    });
  }

  auto default_value = ((BaseDictionary<dis_int>*)somestore->getValueId(3, 0).dict)->getValue(DEFAULT_VID);
  {
    debug("ScanOperator on default storage (default value)", default_value);
    ScanOperator so(somestore.get(), 3, default_value);
    times_measure("dispatch", [&]() {
      so.execute();
    });
    times_measure("fallback", [&]() {
      so.executeFallback();
    });
    times_measure("abstract", [&]() {
      so.executeAbstract();
    });
  }
  auto other_value = ((BaseDictionary<dis_int>*)somestore->getValueId(3, 0).dict)->getValue(DEFAULT_VID) + 1;
  {
    debug("ScanOperator on default storage (other value)", other_value);
    ScanOperator so(somestore.get(), 3, other_value);
    times_measure("dispatch", [&]() {
      so.execute();
    });
    times_measure("fallback", [&]() {
      so.executeFallback();
    });
    times_measure("abstract", [&]() {
      so.executeAbstract();
    });
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
