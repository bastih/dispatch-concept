#include "helpers/debug.hpp"
#include "helpers/measure.h"
#include "access/Generator.h"

#include "access/ScanOperator.h"
#include "access/EmptyOperator.h"
#include "access/FullOperator.h"
#include "access/MaterializingScan.h"
#include "access/JoinScan.h"

#include "storage/structural.h"

int main(int argc, char* const argv[]) {
  debug("Generating");
  auto somestore = makeStore();
  auto smallstore = makeSmallStore();
  somestore->cacheOffsets();
  smallstore->cacheOffsets();
  debug("Done.");

  std::random_device rd;
  std::uniform_int_distribution<int> dist(0, UPPER_VID);
  dis_int value = dist(rd);

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


  {
    debug("JoinScan");
    JoinScan so(somestore.get(), smallstore.get(), col_t(4), col_t(4));
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
