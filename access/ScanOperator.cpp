#include "access/ScanOperator.h"

#include "storage/alltypes.h"
#include "dispatch/Operator.h"

template <typename T>
class ScanOperatorImpl : public OperatorNew<ScanOperatorImpl<T>, all_types_new> {
 public:
  std::vector<std::size_t> positions;
  T needle;
  std::size_t offset;

  template <typename TAB, template <class>  class Dictionary>
  void execute_special(TAB*, FixedStorage* fs, Dictionary<T>* t) {
    value_id_t vid = t->getSubstitute(needle);
    for (std::size_t i = 0, e = fs->rows(); i < e; ++i) {
      if (fs->get(i) == vid) {
        positions.push_back(i + offset);
      }
    }
  }

  template <typename TAB, template <class> class Dictionary>
  void execute_special(TAB*, DefaultValueCompressedStorage* dv, Dictionary<T>* t) {
    value_id_t vid = t->getSubstitute(needle);
    dv->createPositionList(vid, offset, positions);
  }

  void execute_special(ATable*, AStorage* s, ADictionary* d) {
    auto bd = static_cast<BaseDictionary<T>*>(d);
    value_id_t vid = bd->getSubstitute(needle);
    for (std::size_t i = 0, real_pos = offset, e = s->rows(); i < e; ++i, ++real_pos) {
      if (s->get(i) == vid) {
        positions.push_back(real_pos);
      }
    }
  }
};

ScanOperator::ScanOperator(ATable* t, std::size_t column, dis_int value)
    : _table(t), _column(column), _value(value) {}

void ScanOperator::execute() {
  ScanOperatorImpl<dis_int> o;
  o.needle = _value;

  for (const auto& part : _table->getVerticalPartitions(_column)) {
    o.offset = part.start;
    o.execute(const_cast<ATable*>(part.table), 
              const_cast<AStorage*>(part.storage),
              const_cast<ADictionary*>(part.dict));
  }
}

void ScanOperator::executeFallback() {
  ScanOperatorImpl<dis_int> o;
  o.needle = _value;
  for (const auto& part : _table->getVerticalPartitions(_column)) {
    o.offset = part.start;
    o.execute_special(const_cast<ATable*>(part.table), const_cast<AStorage*>(part.storage),
                       const_cast<ADictionary*>(part.dict));
  }
}

void ScanOperator::executeAbstract() {
  std::vector<std::size_t> positions;
  for (std::size_t i = 0, e = _table->height(); i < e; ++i) {
    if (_table->getValue<dis_int>(_column, i) == _value) {
      positions.push_back(i);
    }
  }
}

void ScanOperator::executePerfect() {
  std::vector<size_t> positions;
  assert(_table->getVerticalPartitions(_column).size() == 2);
  for (const auto& part : _table->getVerticalPartitions(_column)) {
    if (part.start == 0) {
      auto fs = static_cast<FixedStorage*>(const_cast<AStorage*>(part.storage));
      auto ds = static_cast<OrderedDictionary<dis_int>*>(const_cast<ADictionary*>(part.dict));
      assert(fs && ds);
      value_id_t vid = ds->getSubstitute(_value);
      for (std::size_t i = 0, real_pos = part.start, e = fs->rows(); i < e; ++i, ++real_pos) {
        if (fs->get(i) == vid) {
          positions.push_back(real_pos);
        }
      }
    } else {
      auto fs = static_cast<FixedStorage*>(const_cast<AStorage*>(part.storage));
      auto us = static_cast<UnorderedDictionary<dis_int>*>(const_cast<ADictionary*>(part.dict));
      assert(fs && us);
      value_id_t vid = us->getSubstitute(_value);
      for (std::size_t i = 0, real_pos = part.start, e = fs->rows(); i < e; ++i, ++real_pos) {
        if (fs->get(i) == vid) {
          positions.push_back(real_pos);
        }
      }
    }
  }
}
