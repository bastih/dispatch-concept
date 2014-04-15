#include <vector>
#include <type_traits>
#include "../dispatch2/dispatch.h"
#include "../dispatch2/foreach.hpp"

Base::~Base() {}

class DBase : public Base {
public:
  virtual size_t method() = 0;
};

{% for num in range(100) %}
class Child{{num}} final : public DBase {
 public:
  virtual size_t method() { return {{num}}; }
};
{% endfor %}

class Operation {
public:
  size_t sum = 0;
    template <typename T>
    void execute(T* db) {
        for (size_t i=0; i < 100; ++i) {
            sum += db->method();
        }
    }
};

using types = std::tuple <
{% for num in nums %}
    std::tuple<Child{{num}}*>{% if not loop.last %},{% endif %}
{% endfor %}
    >;

const size_t INSTANCES =  1000 * 1000;

struct Op {
std::vector<DBase*>& r;
Op(std::vector<DBase*>& v) : r(v) {}
template <typename T>
void operator()(const std::tuple<T>& t)  {
r.push_back(new typename std::remove_pointer<T>::type);
}
};

std::vector<DBase*> createTypes() {

std::vector<DBase*> results;
   Op op(results);
while (results.size() < INSTANCES) {
   for_each(types(), op);
}
results.resize(INSTANCES);
   return results;
}

int main() {


  auto instances = createTypes();
/* perf trace here */
Operation op;
dispatch<types, Operation, void> dp;
  for(const auto& instance: instances)
    dp(op, instance);
/* perf trace here */
}
