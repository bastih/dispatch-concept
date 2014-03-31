#include "dispatch2/dispatch.h"

#include <vector>

class ABase : public Base {
public:
    virtual ~ABase() = default;
    void base() {}
    virtual size_t vm() = 0;
};

class Child1 final : public ABase {
public:
    size_t c1() { return 3; }
    virtual size_t vm() { return c1(); }
};

class Child2 final : public ABase {
public:
    size_t c2() { return 5; }
    virtual size_t vm() { return c2(); }
};

std::vector<ABase*> generate() {
    std::vector<ABase*> result;
    for (int i =0, e=1000; i<e; i++) {
        result.push_back(i % 2 ? (ABase*) new Child1: new Child2);
    }
    return result;
}

class Operation {
public:
    using return_type = void;
    /* alt:
       template <typename C>
       void execute(C* c) {
         c->vm();
       }
     */

    void execute(Child1* c1) {
        c1->Child1::vm();
    }
    void execute(Child2* c1) {
        c1->Child2::vm();
    }
    void execute(ABase* b) {
        b->vm();
    }
};



int main() {
    auto instances = generate();
    Operation op;
#ifdef WITH_DISPATCH
    using classes = std::tuple< std::tuple<Child1*>, std::tuple<Child2*> >;
    dispatch<classes, Operation> dp;
    for (const auto& instance: instances) {
        dp(op, instance);
    }
#else
    for (const auto& instance: instances) {
        if (auto c1 = dynamic_cast<Child1*>(instance)) {
            op.execute(c1);
        } else if (auto c2 = dynamic_cast<Child2*>(instance)) {
            op.execute(c2);
        } else {
            op.execute(instance);
        }
    }
#endif
}
