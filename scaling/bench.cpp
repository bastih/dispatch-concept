#include <vector>
#include <type_traits>
#include "../dispatch2/dispatch.h"
#include "../dispatch2/foreach.hpp"

Base::~Base() {}

class DBase : public Base {
public:
    virtual void method() = 0;
};

{% for num in nums %}
class Child{{num}} : public DBase {
 public:
    virtual void method() { }
};
{% endfor %}

class Operation {
public:
    template <typename T>
    void execute(T* db) {
        db->method();
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
