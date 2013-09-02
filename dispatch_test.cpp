#include "storage.h"
#include "OperatorImpl.h"
#include "debug.hpp"
#include "make_unique.h"
#include <random>
#include <map>
#include "ScanOperator.h"
#include "EmptyOperator.h"
#include "FullOperator.h"
#include "PapiTracer.h"

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
  for (dis_int i=0; i < UPPER_VID; i++)
    d->add(i);
  return d;
}

std::unique_ptr<UnorderedDictionary<dis_int> > makeUnorderedDict() {
  auto d = make_unique<UnorderedDictionary<dis_int> >();
  for (dis_int i=UPPER_VID-1; i >= 0; --i)
    d->add(i); // add vids in reverse
  return d;
}

std::unique_ptr<ATable> makeMainTable() {
  return make_unique<Table>(makeFixedStorage(1000*1000), makeOrderedDict());
}

std::unique_ptr<ATable> makeDeltaTable() {
  return make_unique<Table>(make_unique<FixedStorage>(2*1000*1000), makeUnorderedDict());
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

inline uint64_t rdtsc() {
  uint32_t lo, hi;
  __asm__ __volatile__ (
      "xorl %%eax, %%eax\n"
      "cpuid\n"
      "rdtsc\n"
      : "=a" (lo), "=d" (hi)
      :
      : "%ebx", "%ecx");
  return (uint64_t)hi << 32 | lo;
}

template <typename F>
void times_measure(std::size_t i, F&& f) {
  std::map<std::string, std::vector<std::uint64_t> > values;
  for (std::size_t r=0; r<i; ++r) {
    PapiTracer pt;
    pt.addEvent("PAPI_TOT_CYC");
    pt.addEvent("PAPI_TOT_INS");
    pt.addEvent("PAPI_L1_DCM");
    pt.start();
    f();
    pt.stop();
    values["CYC"].push_back(pt.value("PAPI_TOT_CYC"));
    values["INS"].push_back(pt.value("PAPI_TOT_INS"));
    values["L1M"].push_back(pt.value("PAPI_L1_DCM"));
  }
  for (auto& kv : values) {
    auto& times = kv.second;
    std::sort(times.begin(), times.end());
    std::uint64_t sum = std::accumulate(times.begin(), times.end(), 0u);
    debug(kv.first, "avg", sum / i, "min", times[0], "max", times.back());
  }
}

int main () {
  debug("Generating");
  auto somestore = makeStore();
  debug("Done.");
  auto parts = somestore->getPartitions(1);
  std::random_device rd;
  std::uniform_int_distribution<int> dist(0, UPPER_VID);
  dis_int value = dist(rd);

  {
    debug("EmptyOperator");
    EmptyOperator so(somestore.get(), 1);
    debug("dispatched");
    times_measure(15, [&] () {
        so.execute();
      });
  
    debug("fallback");
    times_measure(15, [&] () {
        so.executeFallback();
      });
  }
  {
    debug("FullOperator");
    FullOperator so(somestore.get(), 1);
    debug("dispatched");
    times_measure(15, [&] () {
        so.execute();
      });
    debug("fallback");
    times_measure(15, [&] () {
        so.executeFallback();
      });
  }

  {
    debug("ScanOperator");
    ScanOperator so(somestore.get(), 1, value);
    debug("dispatched");
    times_measure(15, [&] () {
        so.execute();
      });
    debug("fallback");
    times_measure(15, [&] () {
        so.executeFallback();
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

