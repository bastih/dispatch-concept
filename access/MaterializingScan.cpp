#include "MaterializingScan.h"

#include <cstdint>
#include <string>

#include "storage/alltypes.h"
#include "dispatch2/dispatch.h"


class MatScanOperatorImpl {
 public:
  std::vector<std::string> materialized_row;
  std::size_t row;
  std::size_t col;

  template <typename TAB, typename STORE, typename DICT>
  void execute(TAB* t, STORE* s, DICT* d) {
    materialized_row.push_back(d->getValueString(s->get(row)));
  }
};

MaterializingScanOperator::MaterializingScanOperator(ATable* t, std::size_t row) : _table(t), _row(row) {}


dispatch< product<table_types, storage_types, dictionary_types> , MatScanOperatorImpl, void > matscan_dispatch;

void MaterializingScanOperator::execute() {
  MatScanOperatorImpl o;
  o.materialized_row.reserve(_table->width());
  for (const auto& part : _table->getHorizontalPartitions(_row)) {
    o.row = part.offset;
    o.col = part.start;
    matscan_dispatch(o, const_cast<ATable*>(part.table), const_cast<AStorage*>(part.storage),
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
    o.execute(const_cast<ATable*>(part.table), const_cast<AStorage*>(part.storage),
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
