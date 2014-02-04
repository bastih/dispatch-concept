#pragma once

#include <string>
#include <vector>
#include <algorithm>
#include <map>

#include "PapiTracer.h"
#include "debug.hpp"

struct PerformanceResults {
  std::vector<std::string> names;
  std::map<std::string, std::vector<std::uint64_t> > values;

  PerformanceResults(decltype(names) n, decltype(values) v) : names(n), values(v) {}
};

void print(PerformanceResults results) {
  for (const auto& kv: results.values) {
    for (const auto& value: kv.second){
      for (const auto& name: results.names) {
        std::cout << name << ",";
      }
      std::cout << kv.first << "," << value << std::endl;
    }
  }
}

template <typename F>
void times_measure(std::vector<std::string> names, F&& f, std::size_t i = 40) {
  std::map<std::string, std::vector<std::uint64_t> > values;
  {
    PapiTracer pt1;
    pt1.addEvent("PAPI_TOT_CYC");
    pt1.start();
    f();
    pt1.stop();
    f();
    f();
  }
  for (std::size_t r = 0; r < i; ++r) {
    PapiTracer pt;
    pt.addEvent("PAPI_TOT_CYC");
    pt.addEvent("PAPI_TOT_INS");
    pt.addEvent("PAPI_L1_DCM");
    pt.addEvent("PAPI_L3_TCM");
    pt.start();
    f();
    pt.stop();
    values["CYC"].push_back(pt.value("PAPI_TOT_CYC"));
    values["INS"].push_back(pt.value("PAPI_TOT_INS"));
    values["L1M"].push_back(pt.value("PAPI_L1_DCM"));
    values["L3M"].push_back(pt.value("PAPI_L3_TCM"));
  }

  print(PerformanceResults(names, values));
  //for (auto& kv : values) {
  //auto& times = kv.second;
  //std::sort(times.begin(), times.end());
  //std::uint64_t sum = std::accumulate(times.begin(), times.end(), 0ull);
  //    debug_delim(", ", name, kv.first, "avg", static_cast<std::uint64_t>(sum / i), "min",
   //        times.front(), "max", times.back(), "warmup",
  //         pt1.value("PAPI_TOT_CYC"));
  // }
}
