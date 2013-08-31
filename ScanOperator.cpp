#include "ScanOperator.h"

#include "Operator.h"
#include "storage.h"

template <typename T>
class ScanOperatorImpl : public Operator< ScanOperatorImpl<T> > {
public:
  std::vector<std::size_t> positions;
  T needle;
  std::size_t offset;

  template <typename TAB, typename F, template <class> class Dictionary>
  void execute_special(TAB*, F* fs, Dictionary<T>* t) {
    value_id_t vid = t->getSubstitute(needle);
    for (std::size_t i=0, real_pos=offset, e=fs->rows(); i < e; ++i, ++real_pos) {
      if (fs->get(i) == vid) {
        positions.push_back(real_pos);
      }
    }
  }


  void execute_fallback(ATable*, AStorage* s, ADictionary* d) {
    auto bd = static_cast<BaseDictionary<T>* >(d);
    value_id_t vid = bd->getSubstitute(needle);
    for (std::size_t i=0, real_pos=offset, e=s->rows(); i < e; ++i, ++real_pos) {
      if (s->get(i) == vid) {
        positions.push_back(real_pos);
      }
    }
  }
};


ScanOperator::ScanOperator(ATable* t, std::size_t column, dis_int value) : _table(t), _column(column), _value(value) {}

void ScanOperator::execute() {
  ScanOperatorImpl<dis_int> o;
  o.needle = _value;

  for (const auto& part: _table->getPartitions(_column)) {
    o.offset = part.start;
    o.execute(const_cast<ATable*>(part.table),
              const_cast<AStorage*>(part.storage),
              const_cast<ADictionary*>(part.dict));
  }
}

void ScanOperator::executeFallback() {
  ScanOperatorImpl<dis_int> o;
  o.needle = _value;
  for (const auto& part: _table->getPartitions(_column)) {
    o.offset = part.start;
    o.execute_fallback(const_cast<ATable*>(part.table),
                       const_cast<AStorage*>(part.storage),
                       const_cast<ADictionary*>(part.dict));
  }
}
