#pragma once

#include <array>
#include <stdexcept>
#include "boost/mpl/at.hpp"
#include "dispatch/Typed.h"
#include "dispatch/loop.hpp"
#include "helpers/debug.hpp"
#include "dispatch/tuple_foreach.h"

#define UNUSED __attribute__((unused))

template<typename T, typename... ARGS>
constexpr auto has_execute_overload(UNUSED T* o,
                                    UNUSED ARGS... args) -> decltype(o->execute_special(args...), bool()) { return true; }
constexpr auto has_execute_overload(...) -> bool { return false; }

template <typename T, typename... ARGS>
constexpr auto has_fallback_overload(UNUSED T* o,
                                     UNUSED ARGS... args) -> decltype(o->fallback(args...), bool()) { return true; }
constexpr auto has_fallback_overload(...) -> bool { return false; }


template <class O, class... ARGS>
auto call_uspecial(O* op, ARGS... args)
    -> typename std::enable_if<has_execute_overload((O*) 0, ARGS()...), bool>::type {  
  op->execute_special(args...);
  return true;
}

template <class O, class... ARGS>
inline auto call_uspecial( O* op, ARGS... args)
    -> typename std::enable_if<!has_execute_overload((O*) 0, ARGS()...)
                               and
                               has_fallback_overload((O*) 0, ARGS()...), bool>::type {
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
        tswitch<new_params, LEVEL + 1, (LEVEL+1 == ARGCOUNT)>().call(dispatch_vals, op, reinterpret_cast<new_params*>(params));
      }
    }
  };

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

  

  bool exec;
  template <typename... ARGS>
  void execute(ARGS... args) {
    auto me = static_cast<OperatorType*>(this);
    std::tuple<ARGS...> tuple(args...);
    std::array<std::size_t, ARGCOUNT> dispatch_values;
    populate_dispatch_values<std::tuple<ARGS...> > pop(dispatch_values, tuple);
    loop<0, ARGCOUNT>()(pop);
 
    exec = false;
    tswitch<std::tuple<ARGS...>, 0, 0==ARGCOUNT>().call(dispatch_values, me, &tuple);
    if (!exec) {
      me->execute_fallback(args...);
    }
  }
};
