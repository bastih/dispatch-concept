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

std::vector<std::pair<ABase*, ABase*>> generate() {
    std::vector<std::pair<ABase*, ABase*>> result;
    for (int i =0, e=1000; i<e; i++) {
        result.push_back(std::make_pair(i % 2 ? (ABase*) new Child1: new Child2,
                                        i % 3 ? (ABase*) new Child1: new Child2));
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

    void execute(Child1* c1, Child1* c2) {
        c1->vm();
        c2->vm();
    }

    void execute(Child2* c1, Child1* c2) {
        c1->vm();
        c2->vm();
    }

    void execute(ABase* b, ABase* bb) {
        b->vm();
        bb->vm();
    }
};


int main() {
    auto instances = generate();
    Operation op;
#ifdef WITH_DISPATCH
    using classes = std::tuple< std::tuple<Child1*, Child1*>, std::tuple<Child2*, Child1*> >;
    dispatch<classes, Operation> dp;
    for (const auto& instance: instances) {
        dp(op, instance.first, instance.second);
    }
#else
    for (const auto& instancep: instances) {
        auto i1 = instancep.first;
        auto i2 = instancep.second;
        if (auto c1 = dynamic_cast<Child1*>(i1)) {
            if (auto c2 = dynamic_cast<Child1*>(i2)) {
                op.execute(c1, c2);
            } else {
                op.execute(i1, i2);
            }
        } else if (auto c1 = dynamic_cast<Child2*>(i1)) {
            if (auto c2 = dynamic_cast<Child1*>(i2)) {
                op.execute(c1, c2);
            } else {
                op.execute(i1, i2);
            }
        } else {
            op.execute(i1, i2);
        }
    }
#endif
}
