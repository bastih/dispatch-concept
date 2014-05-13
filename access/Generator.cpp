#include "Generator.h"

#include <random>
#include <set>
#include <iostream>

#include "storage/stores.h"
#include "storage/dicts.h"
#include "storage/structural.h"

using rand_engine = std::mt19937;

std::unique_ptr<BaseDictionary<dis_int>> makeOrderedDict(std::size_t sz=UPPER_VID) {
    auto d = make_unique<OrderedDictionary<dis_int>>();
    for (dis_int i = 0; i <= sz; i++) d->add(i);
    return std::move(d);
}

std::unique_ptr<BaseDictionary<dis_int>> makeUnorderedDict(std::size_t sz=UPPER_VID) {
    auto d = make_unique<UnorderedDictionary<dis_int>>();
    for (dis_int i = sz; i >= 0; --i) d->add(i);  // add vids in reverse
    return std::move(d);
}
/*
std::unique_ptr<BaseDictionary<dis_int>> makeOrderedDictV(const std::set<value_id_t>& values) {
    auto d = make_unique<OrderedDictionary<dis_int>>();
    //for (const auto& val : values) {
    //d->add(val);
    //}
    for (size_t i = 0, e= *values.rbegin()+1; i<e; i++) {
        d->add(i);
    }
    return std::move(d);
}

std::unique_ptr<BaseDictionary<dis_int>> makeUnorderedDictV(const std::set<value_id_t>& values) {
    auto d = make_unique<UnorderedDictionary<dis_int>>();
    for (size_t i = *values.rbegin()+1, e=0; i != e; i--) {
        d->add(i);
    }
    return std::move(d);
}
*/


using table_func = std::function<std::unique_ptr<ATable>()>;
static thread_local rand_engine rd;

template <typename T>
std::unique_ptr<ATable> make(std::vector<table_func> funcs) {
    std::vector<std::unique_ptr<ATable>> ves;
    for (auto&& func : funcs) {
        ves.emplace_back(func());
    }
    return make_unique<T>(std::move(ves));
}

using dist_func = std::function<value_id_t(rand_engine&)>;
using dict_func = std::function<std::unique_ptr<BaseDictionary<dis_int>>(std::size_t)>;

std::unique_ptr<ATable> makeRandomIntFixedTable(AStorage* fs, std::size_t distinct, std::size_t rows, dict_func dict_func, dist_func distf){
    //auto fs = make_unique<FixedStorage>(rows);
    //std::set<value_id_t> values;
    auto dict = dict_func(distinct);
    //std::cout << typeid(*fs).name() << std::endl;
    for (auto i = 0ul; i < rows; ++i) {
        auto val = dict->getSubstitute(distf(rd));
        //std::cout << val << " ";
        //values.insert(val);
        fs->set(i, val);
    }
    //std::cout << std::endl;
    return make_unique<Table>(std::unique_ptr<AStorage>(fs), std::move(dict));
}

std::vector<size_t> makePartOffsets(std::size_t num, std::size_t rows) {
    std::set<size_t> r;
    std::uniform_int_distribution<std::size_t> dist(1, rows);
    r.insert(rows);
    while (r.size() < num) {
        r.insert(dist(rd));
    }
    std::vector<std::size_t> ret;
    std::adjacent_difference(std::begin(r), std::end(r), std::back_inserter(ret));
    return ret;
}

std::unique_ptr<ATable> makeEqualPartitionTable(std::size_t rows, std::size_t cols, std::size_t parts) {
    size_t vids_max = rows / 100;
    auto column_func = [&] () {
        std::vector<table_func> pgs;
        auto unif = [=] (rand_engine& rd) { std::uniform_int_distribution<value_id_t> dist(0, vids_max); return dist(rd); };
        for (size_t part=0; part < parts; ++part) {
            auto offset = rows/parts;
            pgs.push_back(std::bind(makeRandomIntFixedTable, new FixedStorage(offset), vids_max, offset, makeOrderedDict, unif));
        }
        return make<Horizontal>(pgs);
    };
    std::vector<table_func> colfuncs;
    for (std::size_t i=0; i < cols; i++) {
        colfuncs.push_back(column_func);
    }
    auto tab = make<Vertical>(colfuncs);
    //tab->structure(std::cout);
    return tab;
}

std::unique_ptr<ATable> makeCEqualPartitionTable(std::size_t rows, std::size_t cols, std::size_t parts) {
    size_t vids_max = rows / 100;
    auto column_func = [&] () {
        std::vector<table_func> pgs;
        auto geom = [=] (rand_engine& rd) { std::geometric_distribution<> dist(0.8);
                                            if (dist(rd) == 0)
                                                return 0u;
                                            else
                                                return std::uniform_int_distribution<value_id_t>(0, vids_max)(rd); };
        for (size_t part=0; part < parts; ++part) {
            auto offset = rows/parts;
            pgs.push_back(std::bind(makeRandomIntFixedTable, new DefaultValueCompressedStorage(offset, 0), vids_max, offset, makeOrderedDict, geom));
        }
        return make<Horizontal>(pgs);
    };
    std::vector<table_func> colfuncs;
    for (std::size_t i=0; i < cols; i++) {
        colfuncs.push_back(column_func);
    }
    auto tab = make<Vertical>(colfuncs);
    return tab;
}


std::unique_ptr<ATable> makeSomeTable() {
    std::size_t rows = 40;
    std::size_t cols = 1;
    std::size_t num_parts = 2;
    std::size_t vids_max = 30;
    auto column_func = [&] () {
        auto offsets = makePartOffsets(num_parts, rows);
        std::vector<table_func> pgs;
        auto unif = [=] (rand_engine& rd) { std::uniform_int_distribution<value_id_t> dist(0, vids_max); return dist(rd); };
        auto geom = [=] (rand_engine& rd) { std::geometric_distribution<> dist(0.8);
                                            if (dist(rd) == 0)
                                                return 0u;
                                            else
                                                return std::uniform_int_distribution<value_id_t>(0, vids_max)(rd); };
        for (auto offset : offsets) {
            pgs.push_back(std::bind(makeRandomIntFixedTable, new FixedStorage(offset), vids_max, offset, makeOrderedDict, geom));
            pgs.push_back(std::bind(makeRandomIntFixedTable, new DefaultValueCompressedStorage(offset, 0), vids_max, offset, makeOrderedDict, geom));
            pgs.push_back(std::bind(makeRandomIntFixedTable, new FixedStorage(offset), vids_max, offset, makeOrderedDict, unif));
            pgs.push_back(std::bind(makeRandomIntFixedTable, new DefaultValueCompressedStorage(offset, 0), vids_max, offset, makeUnorderedDict, unif));
        }
        return make<Horizontal>(pgs);
    };
    std::vector<table_func> colfuncs;
    for (std::size_t i=0; i < cols; i++) {
        colfuncs.push_back(column_func);
    }
    auto tab = make<Vertical>(colfuncs);
    tab->structure(std::cout);
    return tab;
}

std::unique_ptr<FixedStorage> makeFixedStorage(std::size_t sz) {
  auto fs = make_unique<FixedStorage>(sz);
  std::random_device rd;
  std::uniform_int_distribution<int> dist(0, UPPER_VID);

  for (auto i = 0ul, e = sz; i < e; ++i) {
    fs->set(i, dist(rd));
  }
  return fs;
}

std::unique_ptr<FixedStorage> makeSequentialFixedStorage(std::size_t sz) {
  auto fs = make_unique<FixedStorage>(sz);
  for (auto i = 0ul, e = sz; i < e; ++i) {
    fs->set(i, i % UPPER_VID);
  }
  return fs;
}

std::unique_ptr<DefaultValueCompressedStorage> makeValueCompressedStorage(std::size_t sz) {
  auto fs = make_unique<DefaultValueCompressedStorage>(sz, DEFAULT_VID);
  std::random_device rd;

  std::uniform_int_distribution<int> dist(0, UPPER_VID);
  std::uniform_int_distribution<int> dist_default(0, 20);
  for (auto i = 0ul, e = sz; i < e; ++i) {
    if (dist_default(rd) != 0)  // 95 % same value
      fs->set(i, DEFAULT_VID);
    else
      fs->set(i, dist(rd));
  }
  return fs;
}


std::unique_ptr<ATable> makeSeqMainTable(std::size_t sz=MAINSIZE) {
  return make_unique<Table>(makeSequentialFixedStorage(sz), makeOrderedDict());
}

std::unique_ptr<ATable> makeSeqDeltaTable(std::size_t sz=DELTASIZE, std::size_t offset=MAINSIZE) {
  return make_unique<Table>(makeSequentialFixedStorage(sz), makeUnorderedDict(UPPER_VID));
}

std::unique_ptr<ATable> makeMainTable(std::size_t sz=MAINSIZE) {
  return make_unique<Table>(makeFixedStorage(sz), makeOrderedDict());
}

std::unique_ptr<ATable> makeCompressedMainTable(std::size_t sz=MAINSIZE) {
  return make_unique<Table>(makeValueCompressedStorage(sz), makeOrderedDict(UPPER_VID));
}

std::unique_ptr<ATable> makeDeltaTable(std::size_t sz=DELTASIZE) {
  return make_unique<Table>(make_unique<FixedStorage>(sz), makeUnorderedDict(sz));
}

std::unique_ptr<ATable> makeStore(std::size_t main_size, std::size_t delta_size) {
  std::vector<std::unique_ptr<ATable>> mains, deltas, store_parts;
  for (auto i = 0; i < 3; ++i) {
    mains.emplace_back(makeMainTable(main_size));
    deltas.emplace_back(makeDeltaTable(delta_size));
  }
  mains.emplace_back(makeCompressedMainTable(main_size));
  deltas.emplace_back(makeDeltaTable(delta_size));
  mains.emplace_back(makeSeqMainTable(main_size));
  deltas.emplace_back(makeSeqDeltaTable(delta_size));
  store_parts.emplace_back(make_unique<Vertical>(std::move(mains)));
  store_parts.emplace_back(make_unique<Vertical>(std::move(deltas)));
  return make_unique<Horizontal>(std::move(store_parts));
}

constexpr std::size_t SMALLSIZE = 30;

std::unique_ptr<ATable> makeSmallStore() {
  std::vector<std::unique_ptr<ATable>> mains, deltas, store_parts;
  for (auto i = 0; i < 3; ++i) {
    mains.emplace_back(makeMainTable(SMALLSIZE));
    deltas.emplace_back(makeDeltaTable(SMALLSIZE));
  }
  mains.emplace_back(makeCompressedMainTable(SMALLSIZE));
  deltas.emplace_back(makeDeltaTable(SMALLSIZE));
  mains.emplace_back(makeSeqMainTable(SMALLSIZE));
  deltas.emplace_back(makeSeqDeltaTable(SMALLSIZE));
  store_parts.emplace_back(make_unique<Vertical>(std::move(mains)));
  store_parts.emplace_back(make_unique<Vertical>(std::move(deltas)));
  return make_unique<Horizontal>(std::move(store_parts));
}
