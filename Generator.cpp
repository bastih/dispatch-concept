#include "Generator.h"

#include <random>

#include "storage/stores.h"
#include "storage/dicts.h"
#include "storage/structural.h"

std::unique_ptr<FixedStorage> makeFixedStorage(std::size_t sz) {
  auto fs = make_unique<FixedStorage>(sz);
  std::random_device rd;
  std::uniform_int_distribution<int> dist(0, UPPER_VID);

  for (auto i = 0ul, e = sz; i < e; ++i) {
    fs->set(i, dist(rd));
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

std::unique_ptr<OrderedDictionary<dis_int>> makeOrderedDict() {
  auto d = make_unique<OrderedDictionary<dis_int>>();
  for (dis_int i = 0; i <= UPPER_VID; i++) d->add(i);
  return d;
}

std::unique_ptr<UnorderedDictionary<dis_int>> makeUnorderedDict() {
  auto d = make_unique<UnorderedDictionary<dis_int>>();
  for (dis_int i = UPPER_VID; i >= 0; --i) d->add(i);  // add vids in reverse
  return d;
}

std::unique_ptr<ATable> makeMainTable() {
  return make_unique<Table>(makeFixedStorage(6 * 1000 * 1000), makeOrderedDict());
}

std::unique_ptr<ATable> makeCompressedMainTable() {
  return make_unique<Table>(makeValueCompressedStorage(6 * 1000 * 1000), makeOrderedDict());
}

std::unique_ptr<ATable> makeDeltaTable() {
  return make_unique<Table>(make_unique<FixedStorage>(1000 * 1000), makeUnorderedDict());
}

std::unique_ptr<ATable> makeStore() {
  std::vector<std::unique_ptr<ATable>> mains, deltas, store_parts;
  for (auto i = 0; i < 3; ++i) {
    mains.emplace_back(makeMainTable());
    deltas.emplace_back(makeDeltaTable());
  }
  mains.emplace_back(makeCompressedMainTable());
  deltas.emplace_back(makeDeltaTable());
  store_parts.emplace_back(make_unique<Vertical>(std::move(mains)));
  store_parts.emplace_back(make_unique<Vertical>(std::move(deltas)));
  return make_unique<Horizontal>(std::move(store_parts));
}
