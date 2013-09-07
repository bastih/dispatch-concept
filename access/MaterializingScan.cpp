#include "MaterializingScan.h"

#include <cstdint>
#include <string>

#include "storage/alltypes.h"
#include "dispatch/Operator.h"


class MatScanOperatorImpl : public Operator<MatScanOperatorImpl, all_types> {
 public:
  std::vector<std::string> materialized_row;
  std::size_t row;
  std::size_t col;

  template <typename TAB, typename STORE, typename DICT>
  void execute_special(TAB* t, STORE* s, DICT* d) {
    materialized_row.push_back(std::to_string(d->getValue(s->get(row))));
  }

  void execute_fallback(ATable* t, AStorage* s, ADictionary* d) {
    materialized_row.push_back(d->getValueString(s->get(row)));
  }
};

MaterializingScanOperator::MaterializingScanOperator(ATable* t, std::size_t row) : _table(t), _row(row) {}

void MaterializingScanOperator::execute() {
  MatScanOperatorImpl o;
  o.materialized_row.reserve(_table->width());
  for (const auto& part : _table->getHorizontalPartitions(_row)) {
    o.row = part.offset;
    o.col = part.start;
    o.execute(const_cast<ATable*>(part.table), const_cast<AStorage*>(part.storage),
              const_cast<ADictionary*>(part.dict));
  }
  result = o.materialized_row;
}

void MaterializingScanOperator::executeFallback() {
  MatScanOperatorImpl o;
  o.materialized_row.reserve(_table->width());
  for (const auto& part : _table->getHorizontalPartitions(_row)) {
    o.row = part.offset;
    o.col = part.start;
    o.execute_fallback(const_cast<ATable*>(part.table), const_cast<AStorage*>(part.storage),
                       const_cast<ADictionary*>(part.dict));
  }
  result = o.materialized_row;
}

void MaterializingScanOperator::executeAbstract() {
  std::vector<std::string> mat;
  mat.reserve(_table->width());
  for (std::size_t col = 0, e = _table->width(); col < e; ++col) {
    auto vid_dct = _table->getValueId(col, _row);
    mat.push_back(vid_dct.dict->getValueString(vid_dct.vid));
  }
  result = mat;
}

class MatScanOperatorImplNew : public OperatorNew<MatScanOperatorImplNew, all_types_new> {
 public:
  std::vector<std::string> materialized_row;
  std::size_t row;
  std::size_t col;

  template <typename TAB, typename STORE, typename DICT>
  void execute_special(TAB* t, STORE* s, DICT* d) {
    materialized_row.push_back(std::to_string(d->getValue(s->get(row))));
  }

  void execute_fallback(ATable* t, AStorage* s, ADictionary* d) {
    materialized_row.push_back(d->getValueString(s->get(row)));
  }
};

MaterializingScanOperatorNew::MaterializingScanOperatorNew(ATable* t, std::size_t row) : _table(t), _row(row) {}

void MaterializingScanOperatorNew::execute() {
  MatScanOperatorImplNew o;
  o.materialized_row.reserve(_table->width());
  for (const auto& part : _table->getHorizontalPartitions(_row)) {
    o.row = part.offset;
    o.col = part.start;
    o.execute(const_cast<ATable*>(part.table), const_cast<AStorage*>(part.storage),
              const_cast<ADictionary*>(part.dict));
  }
  result = std::move(o.materialized_row);
}

void MaterializingScanOperatorNew::executeFallback() {
  MatScanOperatorImplNew o;
  o.materialized_row.reserve(_table->width());
  for (const auto& part : _table->getHorizontalPartitions(_row)) {
    o.row = part.offset;
    o.col = part.start;
    o.execute_fallback(const_cast<ATable*>(part.table), const_cast<AStorage*>(part.storage),
                       const_cast<ADictionary*>(part.dict));
  }
  result = std::move(o.materialized_row);
}

void MaterializingScanOperatorNew::executeAbstract() {
  std::vector<std::string> mat;
  mat.reserve(_table->width());
  for (std::size_t col = 0, e = _table->width(); col < e; ++col) {
    auto vid_dct = _table->getValueId(col, _row);
    mat.push_back(vid_dct.dict->getValueString(vid_dct.vid));
  }
  result = std::move(mat);
}

void MaterializingScanOperatorNew::executePerfect() {
  std::vector<std::string> mat;
  mat.reserve(4);
  auto parts = _table->getHorizontalPartitions(_row);
  assert(parts.size() == 4);
  {
    auto col = 0;
    const auto& dict = static_cast<const OrderedDictionary<dis_int>*>(parts[col].dict);
    const auto& fs = static_cast<const FixedStorage*>(parts[col].storage);
    mat.push_back(dict->getValueString(fs->get(_row)));
  }
  {
    auto col = 1;
    const auto& dict = static_cast<const OrderedDictionary<dis_int>*>(parts[col].dict);
    const auto& fs = static_cast<const FixedStorage*>(parts[col].storage);
    mat.push_back(dict->getValueString(fs->get(_row)));
  }
  {
    auto col = 2;
    const auto& dict = static_cast<const OrderedDictionary<dis_int>*>(parts[col].dict);
    const auto& fs = static_cast<const FixedStorage*>(parts[col].storage);
    mat.push_back(dict->getValueString(fs->get(_row)));
  }
  {
    auto col = 3;
    const auto& dict = static_cast<const OrderedDictionary<dis_int>*>(parts[col].dict);
    const auto& fs = static_cast<const DefaultValueCompressedStorage*>(parts[col].storage);
    mat.push_back(dict->getValueString(fs->get(_row)));
  }
  result = std::move(mat);
}
