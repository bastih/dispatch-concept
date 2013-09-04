#include <cstdint>
#include "dispatch_fwd.h"

class ATable;

class MaterializingScanOperator {
public:
  MaterializingScanOperator(ATable* t, std::size_t row);
  void execute();
  void executeFallback();
  void executeAbstract();
private:
  ATable* _table;
  std::size_t _row;
};
