#include "dispatch2/dispatch.h"

#include "gtest/gtest.h"

class Child1 : public Base {};

class Child2 : public Base {};

class Child3 : public Base {};

using t1 = std::tuple< Child1*, Child2*, Child3* >;
using t2 = std::tuple< Child1*, Child2*, Child3* >;


class Op {
public:
  using return_type = size_t;
  size_t execute(Child1* a, Child2* b) { return 1; }
  size_t execute(Child2* b, Child2* a) { return 2;}
  size_t execute(Base*, Base*)         { return 3;}
  template <typename T, typename T2>
  size_t execute(T* a, T2* b) {    return 4;  }
};

TEST(base, test) {
  using my_types = product<t1, t2>;
  dispatch<my_types, Op> dp; 
  Base* c1 = new Child1;
  Base* c2 = new Child2;
  Base* c3 = new Child3;
  
  Op op;

  EXPECT_EQ(dp(op, c1, c2), 1);
  EXPECT_EQ(dp(op, c2, c2), 2);
  EXPECT_EQ(dp(op, c1, c1), 4);
  EXPECT_EQ(dp(op, c2, c1), 4);
  EXPECT_EQ(dp(op, c3, c3), 4);
  EXPECT_EQ(dp(op, c2, c3), 4);
}

TEST(base, no_product) {
  dispatch<std::tuple< std::tuple<Child1*, Child2*> >, Op> dp; // generate dispatch only for one specialization
  Base* c1 = new Child1;
  Base* c2 = new Child2;
  Base* c3 = new Child3;
  Op op;
  
  EXPECT_EQ(dp(op, c1, c2), 1);
  EXPECT_EQ(dp(op, c2, c2), 3);
  EXPECT_EQ(dp(op, c1, c1), 3);
  EXPECT_EQ(dp(op, c2, c1), 3);
  EXPECT_EQ(dp(op, c3, c3), 3);
  EXPECT_EQ(dp(op, c2, c3), 3);

}
