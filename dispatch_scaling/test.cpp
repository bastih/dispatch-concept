#include <vector>
#include <type_traits>
#include "../dispatch2/dispatch.h"
#include "../dispatch2/foreach.hpp"

Base::~Base() {}

class DBase : public Base {
    virtual void method() = 0;
};


class Child0 : public DBase {
    virtual void method() { }
};

class Child1 : public DBase {
    virtual void method() { }
};

class Child2 : public DBase {
    virtual void method() { }
};

class Child3 : public DBase {
    virtual void method() { }
};

class Child4 : public DBase {
    virtual void method() { }
};

class Child5 : public DBase {
    virtual void method() { }
};

class Child6 : public DBase {
    virtual void method() { }
};

class Child7 : public DBase {
    virtual void method() { }
};

class Child8 : public DBase {
    virtual void method() { }
};

class Child9 : public DBase {
    virtual void method() { }
};


class Operation {
public:
    void execute(DBase* db) {
    }
};

using types = std::tuple <

    std::tuple<Child0*>,

    std::tuple<Child1*>,

    std::tuple<Child2*>,

    std::tuple<Child3*>,

    std::tuple<Child4*>,

    std::tuple<Child5*>,

    std::tuple<Child6*>,

    std::tuple<Child7*>,

    std::tuple<Child8*>,

    std::tuple<Child9*>

    >;

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
    for_each(types(), op);
    return results;
}

int main() {
dispatch<types, Operation, void> dp;
  Operation op;
  auto instances = createTypes();
  for(const auto& instance: instances)
    dp(op, instance);
}
