#include "Generator.h"

#include <random>

#include "storage/stores.h"
#include "storage/dicts.h"
#include "storage/structural.h"

constexpr size_t MAINSIZE = 6 * 1000 * 1000;
constexpr size_t DELTASIZE = 2 * 1000 * 1000;

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
    fs->set(i, i);
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

std::unique_ptr<OrderedDictionary<dis_int>> makeOrderedDict(std::size_t sz=UPPER_VID) {
  auto d = make_unique<OrderedDictionary<dis_int>>();
  for (dis_int i = 0; i <= sz; i++) d->add(i);
  return d;
}

std::unique_ptr<UnorderedDictionary<dis_int>> makeUnorderedDict(std::size_t sz=UPPER_VID, std::size_t offset=0) {
  auto d = make_unique<UnorderedDictionary<dis_int>>();
  for (dis_int i = sz; i >= 0; --i) d->add(i+offset);  // add vids in reverse
  return d;
}

std::unique_ptr<ATable> makeSeqMainTable(std::size_t sz=MAINSIZE) {
  return make_unique<Table>(makeSequentialFixedStorage(sz), makeOrderedDict(sz));
}

std::unique_ptr<ATable> makeSeqDeltaTable(std::size_t sz=DELTASIZE, std::size_t offset=MAINSIZE) {
  return make_unique<Table>(makeSequentialFixedStorage(sz), makeUnorderedDict(sz, offset));
}

std::unique_ptr<ATable> makeMainTable(std::size_t sz=MAINSIZE) {
  return make_unique<Table>(makeFixedStorage(sz), makeOrderedDict(sz));
}

std::unique_ptr<ATable> makeCompressedMainTable(std::size_t sz=MAINSIZE) {
  return make_unique<Table>(makeValueCompressedStorage(sz), makeOrderedDict(sz));
}

std::unique_ptr<ATable> makeDeltaTable(std::size_t sz=DELTASIZE) {
  return make_unique<Table>(make_unique<FixedStorage>(sz), makeUnorderedDict(sz));
}

std::unique_ptr<ATable> makeStore() {
  std::vector<std::unique_ptr<ATable>> mains, deltas, store_parts;
  for (auto i = 0; i < 3; ++i) {
    mains.emplace_back(makeMainTable());
    deltas.emplace_back(makeDeltaTable());
  }
  mains.emplace_back(makeCompressedMainTable());
  deltas.emplace_back(makeDeltaTable());
  mains.emplace_back(makeSeqMainTable());
  deltas.emplace_back(makeSeqDeltaTable());
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
