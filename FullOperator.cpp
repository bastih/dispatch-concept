#include "FullOperator.h"

#include "storage.h"
#include "Operator.h"

class FullOperatorImpl final : public Operator< FullOperatorImpl > {
public:
  template <typename TAB, typename F, typename Dictionary>
  void execute_special(TAB* tab, F* fs, Dictionary* t) {}

  void execute_fallback(ATable*, AStorage* s, ADictionary* d) {}
};


FullOperator::FullOperator(ATable* table, std::size_t column) : _table(table), _column(column) {}

void FullOperator::execute() {
  FullOperatorImpl o;
  for (const auto& part: _table->getPartitions(_column)) {
    o.execute(const_cast<ATable*>(part.table),
              const_cast<AStorage*>(part.storage),
              const_cast<ADictionary*>(part.dict));
  }
}

void FullOperator::executeFallback() {
  FullOperatorImpl o;
  for (const auto& part: _table->getPartitions(_column)) {
    o.execute_fallback(const_cast<ATable*>(part.table),
                      const_cast<AStorage*>(part.storage),
                      const_cast<ADictionary*>(part.dict));
  }
}
