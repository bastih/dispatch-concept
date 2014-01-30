#include <iostream>
#include "dispatch2/dispatch.h"

class Child1 : public Base {

};

class Child2 : public Base {
  
};

class Child3 : public Base {
  
};

using t1 = std::tuple< Child1*, Child2*, Child3* >;
using t2 = std::tuple< Child1*, Child2*, Child3* >;


class Op {
public:
  using return_type = size_t;
  size_t execute(Child1* a, Child2* b) { std::cout << "c1, c2" << std::endl; return 1; }
  size_t execute(Child2* b, Child2* a) { std::cout << "c2, c2" << std::endl; return 2;}
  size_t execute(Base*, Base*)         { std::cout << "Base fallback" << std::endl; return 3;}
  template <typename T, typename T2>
  size_t execute(T* a, T2* b) {
    std::cout << "Generated Fallback..." << typeid(*a).name() << " " << typeid(*b).name() << std::endl;
    return 4;
  }
};

using my_types = typename flat_typelists<typename product<t1, t2>::type>::type;
dispatch<my_types, Op> dp;

int main(int argc, const char * argv[])
{
  Base* c1 = new Child1;
  Base* c2 = new Child2;
  Base* c3 = new Child3;
  
  Op op;

  std::cout << dp(op, c1, c2) << std::endl;
  dp(op, c2, c2);
  dp(op, c1, c1);
  dp(op, c2, c1);
  dp(op, c3, c3);
  dp(op, c3, c2);
  return 0;
}
