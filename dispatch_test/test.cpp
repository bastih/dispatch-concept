#include <cassert>

#include "dispatch/Typed.h"
#include "dispatch/Operator.h"

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#undef CHECK
#undef REQUIRE
#define CHECK(ARG) if(!ARG)throw std::runtime_error("ARG failed");
#define REQUIRE(ARG) CHECK(ARG)

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

class Child1 final : public Base {
 public:
  void child1_special() {}
  COMMON
};

class Child2 final : public Base {
 public:
  void child2_special() {}
  COMMON
};

class Child3 final : public Base {
 public:
  void child3_special() {}
  COMMON
};

class Child4 final : public Base {
public:
  void child4_special() {}
  COMMON
};


using tp = std::tuple<std::tuple<Child1, Child2> >;

class SingleDispatchNew : public OperatorNew<SingleDispatchNew, tp> {
 public:
  void execute_special(Child1* c) { c->do_that(); }

  void execute_special(Child2* c) { c->do_that(); }

  void execute_special(Base* c) { c->do_this(); }
};

using multi_types_new =
    std::tuple<std::tuple<Child1, Child2>,
               std::tuple<Child1, Child2, Child3> >;

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

  void execute_special(Base* c, Base* d) {
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
  void execute_special(Base* c, int i) { c->do_this(); }
};

TEST_CASE("new dispatch with extra params", "[dispatch]") {
  Base* c1 = new Child1;
  SingleDispatchExtraParams si;
  si.execute(c1, 10);
  REQUIRE(c1->do_that_calls() == 1);
  REQUIRE(c1->stored_value() == 10);
}

class TemplateDispatch : public OperatorNew<TemplateDispatch, multi_types_new> {
 public:
  template <typename C1T, typename C2T>
  void execute_special(C1T c1, C2T c2) {
    c1->do_that();
    c2->do_this();
  }

  void execute_special(Child1* c1, Child1* c2) {
    c1->do_that();
    c2->do_that();
  }

  void execute_special(Base* a, Base* b) { execute_special(a, b); }
};

class TemplateDispatch2 {
 public:
  using return_type = int;

  template <typename C1T, typename C2T>
  int execute_special(C1T c1, C2T c2) {
    c1->do_that();
    c2->do_this();
    std::cout << "Templ" << std::endl;
    return 1;
  }

  int execute_special(Child1* c1, Child1* c2) {
    c1->do_that();
    c2->do_that();
    std::cout << "T2 " << std::endl;
    return 2;
  }

  int execute_special(Base* a, Base* b) { return execute_special<Base*, Base*>(a, b); }
};


/*TEST_CASE("template dispatch new disp", "[dispatch]") {
  Base* c1 = new Child1;
  Base* c2 = new Child2;
  Base* c3 = new Child3;
  TemplateDispatch2 td2;
  dispatch<multi_types_new>(td2, c1, c2);
  CHECK(c1->do_that_calls() == 1);
  CHECK(c2->do_this_calls() == 1);
  dispatch<multi_types_new>(td2, c1, c1);
  dispatch<multi_types_new>(td2, c3, c3);
  }*/


class UnifiedDispatch : public OperatorNew<UnifiedDispatch, multi_types_new> {
 public:
  template <typename C1T, typename C2T>
  void execute_special(C1T* c1, C2T* c2) {
    c1->do_that();
    c2->do_this();
  }
  void execute_special(Base* a, Base* b) {}
};

TEST_CASE("unified dispatch", "[dispatch]") {
  UnifiedDispatch si;
  {
    Base* c1 = new Child1;
    si.execute(c1, c1);
    CHECK(c1->do_that_calls() == 1);
    CHECK(c1->do_this_calls() == 1);
  }
  {
    Base* c1 = new Child1;
    Base* c4 = new Child4;
    si.execute(c1, c4);
    CHECK(c1->do_that_calls() == 0);
    CHECK(c1->do_this_calls() == 0);
  }
}
