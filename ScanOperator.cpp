#include "ScanOperator.h"

#include "Operator.h"
#include "storage.h"

template <typename T>
class ScanOperatorImpl : public Operator< ScanOperatorImpl<T> > {
public:
  std::vector<std::size_t> positions;
  T needle;

  void scan(FixedStorage* fs, value_id_t subs) {}

  void execute_special(Table*, FixedStorage* fs, OrderedDictionary<T>* t) {
    t->getSubstitute(needle);
  }

  void execute_special(Table*, FixedStorage* fs, UnorderedDictionary<T>* t) {
    t->getSubstitute(needle);
  }

  void execute_fallback(ATable* t, AStorage* s, ADictionary* d) {
  }
};

ScanOperator::ScanOperator(ATable* t, std::size_t column, dis_int value) : _table(t), _column(column), _value(value) {}

void ScanOperator::execute() {
  ScanOperatorImpl<dis_int> o;
  for (const auto& part: _table->getPartitions(_column)) {
    o.execute(const_cast<ATable*>(part.table),
              const_cast<AStorage*>(part.storage),
              const_cast<ADictionary*>(part.dict));
  }
  std::cout << o.positions.size() << std::endl;
}
