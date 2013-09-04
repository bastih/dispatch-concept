#pragma once

#include <string>
#include <vector>
#include <algorithm>
#include <map>

#include "PapiTracer.h"
#include "debug.hpp"

template <typename F>
void times_measure(std::string name, F&& f, std::size_t i=40) {
  std::map<std::string, std::vector<long long> > values;
  PapiTracer pt1;
  pt1.addEvent("PAPI_TOT_CYC");
  pt1.start();
  f();
  pt1.stop();
    f();  f();
  for (std::size_t r=0; r<i; ++r) {
    PapiTracer pt;
    pt.addEvent("PAPI_TOT_CYC");
    //pt.addEvent("PAPI_TOT_INS");
    //pt.addEvent("PAPI_L1_DCM");
    pt.start();
    f();
    pt.stop();
    values["CYC"].push_back(pt.value("PAPI_TOT_CYC"));
    //values["INS"].push_back(pt.value("PAPI_TOT_INS"));
    //values["L1M"].push_back(pt.value("PAPI_L1_DCM"));
  }
  for (auto& kv : values) {
    auto& times = kv.second;
    std::sort(times.begin(), times.end());
    std::uint64_t sum = std::accumulate(times.begin(), times.end(), 0ull);
    debug(name, kv.first, "avg", static_cast<std::uint64_t>(sum / i), "min", times.front(), "max", times.back(), "warmup", pt1.value("PAPI_TOT_CYC"));
  }
}
