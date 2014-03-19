#include "dispatch2/dispatch.h"
#include "classes.h"
#include "helpers/measure.h"

/*
   Need:
   * Empty dispatch
   * Dispatch with increasing number of combinations
   * Dispatch with increasing number of special cases, but only one generic(slow) case
   * Dispatch with extra parameters
 */


class EmptyOperation {
  void execute(ABase* a) { a->method(); }
};


class OneSpecialization {
  void execute(ABase* a) { a->method(); }
  void execute(A1Base* a) { a->method(); }
};

class OneParamOperation {
  template <class A> //enable_if A is ABase?
  void execute(A* a) { a->method(); }
};

class TwoParamOperation {
  template <class A, class B> //enable_if A is ABase?
  void execute(A* a, B* b) {}
};

class ThreeParamOperation {
  template <class A, class B> //enable_if A is ABase?
  void execute(A* a, B* b, C* c) {}
};

int main() {

}
