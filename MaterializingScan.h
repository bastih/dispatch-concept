#include <cstdint>
#include "dispatch_fwd.h"

#include <vector>
class ATable;

class MaterializingScanOperator {
 public:
  MaterializingScanOperator(ATable* t, std::size_t row);
  void execute();
  void executeFallback();
  void executeAbstract();
  std::vector<std::string> result;

 private:
  ATable* _table;
  std::size_t _row;
};
