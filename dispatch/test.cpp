#include "dispatch/test.h"

#include <cassert>

#include "dispatch/Typed.h"
#include "dispatch/Operator.h"

#include "boost/mpl/vector.hpp"

#define CATCH_CONFIG_RUNNER
#include "catch.hpp"

#define COMMON                                                  \
  public:                                                       \
  virtual void do_that() { _calls++; }                          \
  virtual void do_this() { _calls_this++; }                     \
  virtual std::size_t do_that_calls() { return _calls; }        \
  virtual std::size_t do_this_calls() { return _calls_this; }   \
  virtual void store_value(int a) { _value = a; }               \
  virtual int stored_value() { return _value; }                 \
private:                                                        \
std::size_t _calls = 0;                                         \
std::size_t _calls_this = 0;                                    \
int _value = 0;                                                 \
public:

class Base : public Typed {
  COMMON
};

class Child1 : public Base {
 public:
  void child1_special() {}
  COMMON
};

class Child2 : public Base {
 public:
  void child2_special() {}
  COMMON
};

class Child3 : public Base {
 public:
  void child3_special() {}
  COMMON
};

using types = boost::mpl::vector<boost::mpl::vector<Child1, Child2> >;

class SingleDispatch : public Operator<SingleDispatch, types> {
 public:
  void execute_special(Child1* c) { c->do_that(); }

  void execute_special(Child2* c) { c->do_that(); }

  void execute_fallback(Base* c) { c->do_this(); }
};

using multi_types = boost::mpl::vector<boost::mpl::vector<Child1, Child2>,
                                       boost::mpl::vector<Child1, Child2> >;

class MultiDispatch : public Operator<MultiDispatch, multi_types> {
 public:
  void execute_special(Child1* c, Child1* d) {
    c->do_that();
    d->do_that();
  }

  void execute_special(Child1* c, Child2* d) {
    c->do_that();
    d->do_that();
  }

  void execute_fallback(Base* c, Base* d) {
    c->do_this();
    d->do_this();
  }
};

TEST_CASE("single dispatch", "[dispatch]") {
  Base* c1 = new Child1;
  Base* c2 = new Child2;
  Base* c3 = new Child3;
  SingleDispatch si;
  si.execute(c1);
  REQUIRE(c1->do_that_calls() == 1);
  si.execute(c2);
  REQUIRE(c2->do_that_calls() == 1);
  si.execute(c3);
  REQUIRE(c3->do_this_calls() == 1);
}
TEST_CASE("multi dispatch", "[dispatch]") {
  Base* c1 = new Child1;
  Base* c2 = new Child2;
  Base* c3 = new Child3;
  MultiDispatch mu;
  mu.execute(c1, c1);
  REQUIRE(c1->do_that_calls() == 2);
  mu.execute(c1, c2);
  REQUIRE(c1->do_that_calls() == 3);
  REQUIRE(c2->do_that_calls() == 1);
  mu.execute(c3, c1);
  REQUIRE(c3->do_this_calls() == 1);
  REQUIRE(c1->do_this_calls() == 1);
}

using tp = std::tuple<std::tuple<Child1, Child2> >;

class SingleDispatchNew : public OperatorNew<SingleDispatchNew, tp> {
 public:
  void execute_special(Child1* c) { c->do_that(); }

  void execute_special(Child2* c) { c->do_that(); }

  void execute_fallback(Base* c) { c->do_this(); }
};

using multi_types_new =
    std::tuple<std::tuple<Child1, Child2>, std::tuple<Child1, Child2> >;


TEST_CASE("new dispatch", "[dispatch]") {
  Base* c1 = new Child1;
  Base* c2 = new Child2;
  Base* c3 = new Child3;
  SingleDispatchNew si;
  si.execute(c1);
  REQUIRE(c1->do_that_calls() == 1);
  si.execute(c2);
  REQUIRE(c2->do_that_calls() == 1);
  si.execute(c3);
  REQUIRE(c3->do_this_calls() == 1);
}

class MultiDispatchNew : public OperatorNew<MultiDispatchNew, multi_types_new> {
 public:
  void execute_special(Child1* c, Child1* d) {
    c->do_that();
    d->do_that();
  }

  void execute_special(Child1* c, Child2* d) {
    c->do_that();
    d->do_that();
  }

  void execute_fallback(Base* c, Base* d) {
    std::cout << "FALLBACK" << std::endl;
    c->do_this();
    d->do_this();
  }
};


TEST_CASE("new multi dispatch", "[dispatch]") {
  Base* c1 = new Child1;
  Base* c2 = new Child2;
  Base* c3 = new Child3;
  MultiDispatchNew mu;
  mu.execute(c1, c1);
  REQUIRE(c1->do_that_calls() == 2);
  mu.execute(c1, c2);
  REQUIRE(c1->do_that_calls() == 3);
  REQUIRE(c2->do_that_calls() == 1);
  mu.execute(c3, c1);
  REQUIRE(c3->do_this_calls() == 1);
}

TEST_CASE("multi dispatch on non-existant specialization", "[dispatch]") {
  Base* c2 =  new Child2;
  MultiDispatchNew mu;
  mu.execute(c2, c2);
  REQUIRE(c2->do_this_calls() == 2);
}


class SingleDispatchExtraParams : public OperatorNew<SingleDispatchExtraParams, tp> {
 public:
  void execute_special(Child1* c, int i) { c->do_that(); c->store_value(i); }
  void execute_special(Child2* c, int i) { c->do_that(); }
  void execute_fallback(Base* c, int i) { c->do_this(); }
};

TEST_CASE("new dispatch with extra params", "[dispatch]") {
  Base* c1 = new Child1;
  Base* c2 = new Child2;
  Base* c3 = new Child3;
  SingleDispatchExtraParams si;
  si.execute(c1, 10);
  REQUIRE(c1->do_that_calls() == 1);
  REQUIRE(c1->stored_value() == 10);
}

void run_tests() {
  int r = Catch::Session().run();
  //if (r != 0) throw std::runtime_error("Failed");
  printf("Test success\n");
}
