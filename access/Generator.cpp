#include "Generator.h"

#include <random>
#include <set>

#include "storage/stores.h"
#include "storage/dicts.h"
#include "storage/structural.h"


/*class TableGenerator {
public:
    virtual ~TableGenerator() = default;
    virtual std::unique_ptr<ATable> generate() const;
};

class FixedStorageTableGen : public TableGenerator {
public:
    FixedStorageGen(std::size_t cols, std::size_t rows, )
    }*/

std::unique_ptr<BaseDictionary<dis_int>> makeOrderedDict(std::size_t sz=UPPER_VID) {
    auto d = make_unique<OrderedDictionary<dis_int>>();
    for (dis_int i = 0; i <= sz; i++) d->add(i);
    return std::move(d);
}

std::unique_ptr<BaseDictionary<dis_int>> makeUnorderedDict(std::size_t sz=UPPER_VID, std::size_t offset=0) {
    auto d = make_unique<UnorderedDictionary<dis_int>>();
    for (dis_int i = sz; i >= 0; --i) d->add(i+offset);  // add vids in reverse
    return std::move(d);
}

std::unique_ptr<BaseDictionary<dis_int>> makeOrderedDict(std::set<dis_int>& values) {
    auto d = make_unique<OrderedDictionary<dis_int>>();
    for (const auto& val : values) {
        d->add(val);
    }
    //for (dis_int i = 0; i <= sz; i++) d->add(i);
    return std::move(d);
}

std::unique_ptr<BaseDictionary<dis_int>> makeUnorderedDict(std::set<dis_int>& values) {
    auto d = make_unique<UnorderedDictionary<dis_int>>();
    //for (dis_int i = sz; i >= 0; --i) d->add(i+offset);  // add vids in reverse
    std::vector<dis_int> vals(values.begin(), values.end());
    std::random_shuffle(vals.begin(), vals.end());
        for (const auto& val : vals) {
            d->add(val);
        }
    return std::move(d);
}


using table_func = std::function<std::unique_ptr<ATable>()>;
std::random_device rd;

template <typename T>
std::unique_ptr<ATable> make(std::vector<table_func> funcs) {
    std::vector<std::unique_ptr<ATable>> ves;
    for (auto&& func : funcs) {
        ves.emplace_back(func());
    }
    return make_unique<T>(std::move(ves));
}

std::unique_ptr<ATable> makeRandomIntFixedTable(std::size_t rows, std::size_t max_value, bool ordered){
    auto fs = make_unique<FixedStorage>(rows);
    std::uniform_int_distribution<int> dist(0, max_value);
    std::set<dis_int> values;
    for (auto i = 0ul; i < rows; ++i) {
        auto val = dist(rd);
        auto it = values.insert(val);
        fs->set(i, std::distance(std::begin(values), it.first));
    }
    std::unique_ptr<BaseDictionary<dis_int>> dict =
        ordered ? makeOrderedDict(values)
                : makeUnorderedDict(values);
    return make_unique<Table>(std::move(fs), std::move(dict));
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

std::unique_ptr<ATable> makeSomeTable() {
    std::size_t rows = 80;
    std::size_t num_parts = 60;
    std::size_t vids_max = 30;
    auto somerows = [&] () {
        auto offsets = makePartOffsets(num_parts, rows);
        std::vector<table_func> pgs;
        for (auto offset : offsets) {
            pgs.push_back(std::bind(makeRandomIntFixedTable, /*rows*/ offset, /*vids*/ vids_max, true));
        }
        return make<Horizontal>(pgs);
    };
    return make<Vertical>( { somerows, somerows, somerows  });
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
  return make_unique<Table>(makeSequentialFixedStorage(sz), makeUnorderedDict(UPPER_VID, offset));
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
