#include <vector>
#include <type_traits>
#include <cstdlib>
#include <chrono>
#include <stdexcept>
#include <iostream>
#ifdef WITH_DISPATCH
#include "../dispatch2/dispatch.h"
Base::~Base() {}
#endif

class DBase
#ifdef WITH_DISPATCH
    : public Base
#endif
{
public:
    virtual size_t method() = 0;
};

{% for num in range(200) %}
class Child{{num}} final : public DBase {
 public:
    virtual size_t method() {
        return {{num}};
    }
                   };
{% endfor %}

class Operation {
public:
    size_t sum = 0;
    template <typename T>
    void execute(T* db) {
#ifdef WITH_SIGNIFICANT_WORK
        for (size_t i=0; i < 100; ++i)
#endif
        {
            sum += db->method();
        }
    }
};

const size_t INSTANCES =  1000 * 100;

std::vector<DBase*> createTypes() {
    std::vector<DBase*> results;
    results.reserve(INSTANCES);
    for (size_t i=0; i < INSTANCES; ++i) {
        {% for num in range(nums|last+1) %}
        {% if not loop.first %} else {% endif %}
        if (i % {{nums|last + 1}} == {{num}}) {
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
#ifdef WITH_DISPATCH
    using types = std::tuple <
        {% for num in nums %}
    std::tuple<Child{{num}}*>{% if not loop.last %},{% endif %}
    {% endfor %}
    >;
    dispatch<types, Operation, void> dp;
#endif
    Operation op;
    auto instances = createTypes();
    /* perf trace here */
    auto start = std::chrono::high_resolution_clock::now();
       for(const auto& instance: instances) {
#ifndef WITH_SIGNIFICANT_WORK
           for (size_t i=0; i < 100; ++i) {
#endif
#ifdef WITH_DISPATCH
           dp(op, instance);
#else
           op.execute(instance); // calls as DBase*
#endif
#ifndef WITH_SIGNIFICANT_WORK
           }
#endif // SIGNIFICANT

       }
    auto stop = std::chrono::high_resolution_clock::now();
    std::cout << std::chrono::duration_cast<std::chrono::microseconds>(stop-start).count() << '\n';
    /* perf trace here */
    //printf("%d\n", op.sum);
}
