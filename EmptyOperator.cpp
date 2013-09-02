#include "EmptyOperator.h"

#include "storage.h"
#include "Operator.h"

class EmptyOperatorImpl final : public Operator< EmptyOperatorImpl > {
 public:
  // NO OVERLOADS
  void execute_fallback(ATable*, AStorage* s, ADictionary* d) {}
};


EmptyOperator::EmptyOperator(ATable* table, std::size_t column) : _table(table), _column(column) {}

void EmptyOperator::execute() {
  EmptyOperatorImpl o;
  for (const auto& part: _table->getPartitions(_column)) {
    o.execute(const_cast<ATable*>(part.table),
              const_cast<AStorage*>(part.storage),
              const_cast<ADictionary*>(part.dict));
  }
}

void EmptyOperator::executeFallback() {
  EmptyOperatorImpl o;
  for (const auto& part: _table->getPartitions(_column)) {
    o.execute_fallback(const_cast<ATable*>(part.table),
                      const_cast<AStorage*>(part.storage),
                      const_cast<ADictionary*>(part.dict));
  }
}
