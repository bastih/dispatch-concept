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


template <class OP, class... DISPATCH_ARGS>
auto call_uspecial(OP* op, DISPATCH_ARGS... args)
    -> typename std::enable_if<has_special((OP*)0, std::forward<DISPATCH_ARGS>(nullptr)...),
                               void>::type {
  op->execute_special(args...);
}



template <class T>
void debug_types(T a) {
  std::cout << typeid(T).name() << std::endl;
}


template <class T, class... Ts>
void debug_types(T a, Ts... args) {
  std::cout << typeid(T).name() << std::endl;
}

// typename std/::enable_if<!HasExecuteSpecial<OP, void, TABLE*, STORAGE*,
// DICT*>::value, int>::type = 0>
// Is valid when there is no viable overload in op for the given
// params -- don't do anything, there is no match here
template <class OP, class... DISPATCH_ARGS>
auto call_uspecial(OP* op, DISPATCH_ARGS... args)
    -> typename std::enable_if<
      not has_special((OP*)0, std::forward<DISPATCH_ARGS>(nullptr)...),
      void>::type { std::cout << "this sucks" << std::endl; debug_types(args...); }


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


#include <tuple>
#include "tuple_foreach.h"

template<int...> struct index_tuple{};

template<int I, typename IndexTuple, typename... Types>
struct make_indexes_impl;

template<int I, int... Indexes, typename T, typename ... Types>
struct make_indexes_impl<I, index_tuple<Indexes...>, T, Types...>
{
  typedef typename make_indexes_impl<I + 1, index_tuple<Indexes..., I>, Types...>::type type;
};

template<int I, int... Indexes>
struct make_indexes_impl<I, index_tuple<Indexes...> >
{
  typedef index_tuple<Indexes...> type;
};

template<typename ... Types>
struct make_indexes : make_indexes_impl<0, index_tuple<>, Types...>
{};

template<class Ret, class... Args, int... Indexes >
Ret apply_helper( Ret (*pf)(Args...), index_tuple< Indexes... >, std::tuple<Args...>&& tup)
{
  return pf( std::forward<Args>( std::get<Indexes>(tup))... );
}

template<class Ret, class ... Args>
Ret apply(Ret (*pf)(Args...), const std::tuple<Args...>&  tup)
{
  return apply_helper(pf, typename make_indexes<Args...>::type(), std::tuple<Args...>(tup));
}

template<class Ret, class ... Args>
Ret apply(Ret (*pf)(Args...), std::tuple<Args...>&&  tup)
{
  return apply_helper(pf, typename make_indexes<Args...>::type(), std::forward<std::tuple<Args...>>(tup));
}

template <typename OperatorType, typename TYPES>
class OperatorNew {
 public:
  virtual ~OperatorNew() = default;

  template <typename PROCESSED, typename REMAINING, int LEVEL, int ARGCOUNT>
  struct branch_it {
    const std::array<std::size_t, ARGCOUNT>& dispatch_vals;
    OperatorType* op;
    PROCESSED processed;
    REMAINING remaining;

    branch_it(const std::array<std::size_t, ARGCOUNT>& d, OperatorType* o, PROCESSED p, REMAINING r) : dispatch_vals(d), op(o), processed(p), remaining(r) {}

    template <typename T>
    void operator()(const T& x) {
      if (typeId<T>() == std::get<LEVEL>(dispatch_vals)) {
        auto tuple = std::tuple_cat(processed, std::make_tuple(static_cast<T*>(std::get<0>(remaining))));
        auto tail = tuple_tail(remaining);
        tswitch<decltype(tuple), decltype(tail), ARGCOUNT, LEVEL+1>().call(dispatch_vals, op, tuple, tail);
      }
    }
  };

  template <class PROCESSED_PARAMS,
            class REMAINING_ARGS,
            int ARGCOUNT = (std::tuple_size<PROCESSED_PARAMS>::value + std::tuple_size<REMAINING_ARGS>::value),
            int LEVEL = 0,
            bool STOP = (std::tuple_size<REMAINING_ARGS>::value == 0)>
  struct tswitch;

  template <class PROCESSED,
            class REMAINING,
            int ARGCOUNT,
            int LEVEL,
            bool STOP>
  struct tswitch {
    void call(const std::array<size_t, ARGCOUNT>& d, OperatorType* op, PROCESSED pargs, REMAINING args) {
      branch_it<PROCESSED, REMAINING, LEVEL, ARGCOUNT> bit(d, op, pargs, args);
      using tuple_t = typename std::tuple_element<LEVEL, TYPES>::type;
      for_each(*(tuple_t*) nullptr, bit);
    }
  };

  template <class PROCESSED_PARAMS,
            class REMAINING_ARGS,
            int ARGCOUNT,
            int LEVEL>
  struct tswitch<PROCESSED_PARAMS, REMAINING_ARGS, ARGCOUNT, LEVEL, true> {
    void call(const std::array<size_t, ARGCOUNT>&, OperatorType* op, PROCESSED_PARAMS pargs, REMAINING_ARGS args) {
      auto tuple = std::tuple_cat(std::make_tuple(op), pargs, args);
      apply<void>(call_uspecial, tuple);
      throw ImplementationFound();
    }
  };


  template <typename... ARGS>
  void execute(ARGS... args) {
    auto me = static_cast<OperatorType*>(this);
    std::array<std::size_t, sizeof...(ARGS)> dispatch_values {args->getTypeId()...};
    try {
      tswitch<std::tuple<>, std::tuple<ARGS...> >().call(dispatch_values, me, std::tuple<>(), std::forward_as_tuple(args...));
    } catch (const ImplementationFound&) {
      return;
    }
    static_cast<OperatorType*>(this)->execute_fallback(args...);
  }
};
