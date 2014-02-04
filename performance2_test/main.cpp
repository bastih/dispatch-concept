#include "helpers/debug.hpp"
#include "helpers/measure.h"
#include "access/Generator.h"

#include "access/ScanOperator.h"
#include  "access/MaterializingScan.h"
#include "access/JoinScan.h"
#include "storage/structural.h"


int main(int argc, char* const argv[]) {
  //debug("Generating");
  auto somestore = makeStore();
  auto smallstore = makeSmallStore();
  somestore->cacheOffsets();
  smallstore->cacheOffsets();
  //debug("Done.");

  std::random_device rd;
  std::uniform_int_distribution<int> dist(0, UPPER_VID);
  dis_int value = dist(rd);

  {
    std::string mo {"MaterializingScanOperator"};
    MaterializingScanOperator so(somestore.get(), 3);
    times_measure({mo, "dispatch"}, [&so]() {
        so.execute();
      });
    times_measure({mo, "fallback"}, [&so]() {
        so.executeFallback();
      });
    times_measure({mo, "abstract"}, [&so]() {
        so.executeAbstract();
      });
      }

  {
    std::string mo { "Scan FixedLengthStorage" };
    ScanOperator so(somestore.get(), 1, value);
    times_measure({mo, "dispatch"}, [&]() {
        so.execute();
      });
    times_measure({mo, "fallback"}, [&]() {
        so.executeFallback();
      });
    times_measure({mo, "abstract"}, [&]() {
        so.executeAbstract();
      });
    times_measure({mo, "perfect"}, [&]() {
        so.executePerfect();
      });
  }

  auto default_value = ((BaseDictionary<dis_int>*)somestore->getValueId(3, 0).dict)->getValue(DEFAULT_VID);
  {
    std::string mo { "Scan DefaultValueCompressed"};
    ScanOperator so(somestore.get(), 3, default_value);
    times_measure({mo, "dispatch"}, [&]() {
        so.execute();
      });
    times_measure({mo, "fallback"}, [&]() {
        so.executeFallback();
      });
    times_measure({mo, "abstract"}, [&]() {
        so.executeAbstract();
      });
  }
  auto other_value = ((BaseDictionary<dis_int>*)somestore->getValueId(3, 0).dict)->getValue(DEFAULT_VID) + 1;
  {
    std::string mo { "Scan DefaultValueCompressed (non-compressed)"};
    ScanOperator so(somestore.get(), 3, other_value);
    times_measure({mo, "dispatch"}, [&]() {
        so.execute();
      });
    times_measure({mo, "fallback"}, [&]() {
        so.executeFallback();
      });
    times_measure({mo, "abstract"}, [&]() {
        so.executeAbstract();
      });
  }
  {
    std::string mo { "Join"};
    JoinScan so(somestore.get(), smallstore.get(), col_t(4), col_t(4));
    times_measure({mo, "dispatch"}, [&]() {
      so.execute();
    });
    times_measure({mo, "fallback"}, [&]() {
        so.executeFallback();
      });
    times_measure({mo, "abstract"}, [&]() {
        so.executeAbstract();
      });

  }

}
