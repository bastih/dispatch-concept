#pragma once

#include <array>
#include <stdexcept>
#include "boost/mpl/at.hpp"
#include "cartesian.hpp"
#include "dispatch/Typed.h"
#include "helpers/debug.hpp"

// Called by SFINAE if reserve does not exist or is not accessible
template <typename TP, typename... ARGS>
constexpr auto has_special(__attribute__((unused)) TP t,
                           __attribute__((unused)) ARGS... args)
    -> decltype(t -> execute_special(std::forward<ARGS>(args)...), bool()) {
  return true;
}

// Used as fallback when SFINAE culls the template method
template <typename... ARGS>
constexpr bool has_special(ARGS...) {
  return false;
}

template <typename... ARGS>
bool matchingTypeIds() {
  return true;
}

template <class A, class... DISPATCH_ARGS>
bool matchingTypeIds(const Typed* a, DISPATCH_ARGS... args) {
  return ((a->getTypeId() == typeId<typename std::remove_pointer<A>::type>()) &&
          matchingTypeIds<DISPATCH_ARGS...>(args...));
}

class ImplementationFound {};

// Call specialized implementation when available
// Just slightly ugly: enable_if is valid when: op.execute_special(...) is
// overloaded exactly for tall params
// through this specialization, we don't need a fallback execute_special for
// abstract base classes
/* restricting unnamed parameter */
// typename std::enable_if<has_special(HasExecuteSpecial<OP, void, TABLE*,
// STORAGE*, DICT*>::value, int>::type = 0>

template <class OP, class... DISPATCH_ARGS>
auto call_special(OP& op, DISPATCH_ARGS... args)
    -> typename std::enable_if<has_special((OP*)0, std::forward<DISPATCH_ARGS>(nullptr)...),
              void>::type {
  /// extracting table/store/dict actual typeIds through virtual function calls
  /// and compare to what we need for thise combination of types
  // op.checks++;
  if (matchingTypeIds<DISPATCH_ARGS...>(args...)) {
    op.execute_special(args...);
    // UGLY: Exceptions for control flow
    throw ImplementationFound();
  }
}

// typename std/::enable_if<!HasExecuteSpecial<OP, void, TABLE*, STORAGE*,
// DICT*>::value, int>::type = 0>
// Is valid when there is no viable overload in op for the given
// params -- don't do anything, there is no match here
template <class OP, class... DISPATCH_ARGS>
auto call_special(OP& op, DISPATCH_ARGS... args)
    -> typename std::enable_if<
      not has_special((OP*)0, std::forward<DISPATCH_ARGS>(nullptr)...),
      void>::type {}


template <int...>
struct seq {};

template <int N, int... S>
struct gens : gens<N - 1, N - 1, S...> {};

template <int... S>
struct gens<0, S...> {
  typedef seq<S...> type;
};

template <class OP, int N>
struct choose_special {
  OP& op;
  std::array<Typed*, N> args;

  choose_special(OP& op, std::array<Typed*, N> arr) : op(op), args(arr) {}

  template <typename SEQUENCE>
  inline void operator()() {
    call<SEQUENCE>(typename gens<N>::type());
  }

  template <typename SEQ, int... S>
  inline void call(seq<S...>) {
    call_special(op, static_cast<typename boost::mpl::at_c<SEQ, S>::type*>(std::get<S>(args))...);
  }
};


typedef boost::mpl::vector<> empty_;
typedef boost::mpl::vector<empty_, empty_, empty_> empty_types;

#include "boost/mpl/size.hpp"

template <typename OperatorType, typename TYPES>
class Operator {
 public:
  virtual ~Operator() {}

  template <typename... ARGS>
  void execute(ARGS... args) {
    choose_special<OperatorType, boost::mpl::size<TYPES>::value> ci (*static_cast<OperatorType*>(this), {args...});

    try {
      // generates the cartesian product of all types and per
      // combination SEQUENCE, invokes choose_impl<SEQUENCE>()
      boost::mpl::cartesian_product<TYPES>(ci);
    }
    catch (const ImplementationFound&) {
      return;
    }

    static_cast<OperatorType*>(this)->execute_fallback(args...);
  }
};
