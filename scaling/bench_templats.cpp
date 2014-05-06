#include <vector>
#include <type_traits>
#ifdef WITH_DISPATCH
#include "../dispatch2/dispatch.h"
#include "../dispatch2/foreach.hpp"
#endif WITH_DISPATCH

Base::~Base() {}

class DBase :
#ifdef WITH_DISPATCH
public Base
#endif
{
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
    //template <typename T>
    void execute(T* db) {
        sum += db->method();
    }
};

#ifdef WITH_DISPATCH
using types = std::tuple <
{% for num in nums %}
    std::tuple<Child{{num}}*>{% if not loop.last %},{% endif %}
{% endfor %}
    >;
#endif

const size_t INSTANCES =  1000 * 1000;

std::vector<DBase*> createTypes() {
 std::vector<DBase*> results;
 results.reserve(INSTANCES);
 for (size_t i=0; i < INSTANCES; ++i) {
     {% for num in range(max(nums)) %}
     {% if not loop.first %} else {% else %}
     if (i % {{nums|max}} == {{num}}) {
         results.push_back(new Child{{num}});
     }
     {% endfor %}
     else {
         throw std::runtime_error("this should not happen.");
     }
 }
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
