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
    -> typename std::enable_if<
          has_special((OP*)0, std::forward<DISPATCH_ARGS>(nullptr)...),
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
    call_special(op, static_cast<typename boost::mpl::at_c<SEQ, S>::type*>(
                         std::get<S>(args))...);
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
    choose_special<OperatorType, boost::mpl::size<TYPES>::value> ci(
        *static_cast<OperatorType*>(this), {args...});

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


#include "dispatch/tuple_foreach.h"

#define UNUSED __attribute__((UNUSED))

struct NotFound : public std::runtime_error { NotFound(std::string what): std::runtime_error(what) {} };

template<typename T, typename... ARGS>
constexpr auto has_execute_overload(__attribute__((unused)) T* o,
                                    __attribute__((unused)) ARGS... args) -> decltype(o->execute_special(args...), bool()) { return true; }
constexpr auto has_execute_overload(...) -> bool { return false; }

template <typename T, typename... ARGS>
constexpr auto has_fallback_overload(__attribute__((unused)) T* o,
                                     __attribute__((unused)) ARGS... args) -> decltype(o->fallback(args...), bool()) { return true; }
constexpr auto has_fallback_overload(...) -> bool { return false; }


template <class O, class... ARGS>
auto call_uspecial(O* op, ARGS... args)
    -> typename std::enable_if<has_execute_overload((O*) 0, ARGS()...), bool>::type {
  //std::cout << "Special..." << type_names<ARGS...>() << std::endl;
  op->execute_special(args...);
  return true;
}

template <class O, class... ARGS>
inline auto call_uspecial( O* op, ARGS... args)
    -> typename std::enable_if<!has_execute_overload((O*) 0, ARGS()...)
                               and
                               has_fallback_overload((O*) 0, ARGS()...), bool>::type {
  //std::cout << "Fallback..." << type_names<ARGS...>() << std::endl;
  op->execute_fallback(args...);
  return true;
}

template <class O, typename... ARGS>
auto call_uspecial(O* , ARGS... ) -> typename std::enable_if<!has_execute_overload((O*) 0, ARGS()...)
                                                             and
                                                             !has_fallback_overload((O*) 0, ARGS()...), bool>::type {
  std::runtime_error("All bets are off: Could not find a matching fallback for (" + type_names<ARGS...>() + ")");
  return false;
}



template <typename OperatorType, typename TYPES>
class OperatorNew {
  static const int ARGCOUNT = std::tuple_size<TYPES>::value;
  typedef std::array<std::size_t, ARGCOUNT> type_id_array_t;
 public:
  virtual ~OperatorNew() = default;

  template <typename PARAMS, int LEVEL>
  struct branch_it {
    const type_id_array_t& dispatch_vals;
    OperatorType* op;
    PARAMS* params;

    branch_it(const type_id_array_t& d, OperatorType* o, PARAMS* p)
        : dispatch_vals(d), op(o), params(p) {}

    template <typename T>
    void operator()(const T& x) {
      if (typeId<T>() == std::get<LEVEL>(dispatch_vals)) {
        using new_params = typename element_replace<PARAMS, LEVEL, T*>::type;
        tswitch<new_params, LEVEL + 1>().call(dispatch_vals, op, reinterpret_cast<new_params*>(params));
      }
    }
  };

  template <class PARAMS, int LEVEL = 0, bool STOP = (LEVEL == ARGCOUNT)>
  struct tswitch;

  template <class PARAMS, int LEVEL, bool STOP>
  struct tswitch {
    void call(const type_id_array_t& d, OperatorType* op, PARAMS* params) {
      using tuple_t = typename std::tuple_element<LEVEL, TYPES>::type;
      for_each(*(tuple_t*)nullptr, branch_it<PARAMS, LEVEL>(d, op, params));
    }
  };

  template <class PARAMS, int LEVEL>
  struct tswitch<PARAMS, LEVEL, true> {
    void call(const type_id_array_t&, OperatorType* op, PARAMS* params) {
      auto tuple = std::tuple_cat(std::make_tuple(op), *params);
      op->exec = apply<bool>(call_uspecial, tuple);
    }
  };

  template <typename Tuple>
  struct populate_dispatch_values {
    type_id_array_t& arr;
    Tuple& tuple;

    populate_dispatch_values(type_id_array_t& t, Tuple& tup) : arr(t), tuple(tup) {}

    template <int I>
    void operator()() {
      arr[I] = std::get<I>(tuple)->getTypeId();
    }
  };

  template <int I, int E, bool STOP = (I==E)>
  struct loop;

  template <int I, int E>
  struct loop<I, E, true> {
    template <typename F>
    void operator()(F) {}
  };

  template <int I, int E, bool STOP>
  struct loop {
    template<typename F>
    void operator()(F f) {
      f.template operator()<I>();
      loop<I+1, E>()(f);
    }
  };
  
  bool exec;
  template <typename... ARGS>
  void execute(ARGS... args) {
    auto me = static_cast<OperatorType*>(this);
    std::tuple<ARGS...> tuple(args...);
    std::array<std::size_t, ARGCOUNT> dispatch_values;
    populate_dispatch_values<std::tuple<ARGS...> > pop(dispatch_values, tuple);
    loop<0, ARGCOUNT>()(pop);
 
    exec = false;
    tswitch<std::tuple<ARGS...>>().call(dispatch_values, me, &tuple);
    if (!exec) {
      me->execute_fallback(args...);
    }
  }
};
