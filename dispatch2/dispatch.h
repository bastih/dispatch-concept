#pragma once

#include <functional>
#include <tuple>
#include <typeindex>
#include <type_traits>
#include <unordered_map>

#include "Base.h"

#include "tuple_hash.hpp"
#include "cartesian.hpp"
#include "foreach.hpp"
#include "repeat.hpp"

template <class RETURN_TYPE, class O, class... ARGS>
RETURN_TYPE call_special(O* op, ARGS... args) {
  return op->execute(args...);
}

template <typename C>
inline std::tuple<std::type_index> extract_type(C* ptr) {
  return std::make_tuple(std::type_index(typeid(*ptr)));
}

inline std::tuple<> extract_type(...) {
  return {};
}

template <typename... T>
auto extract_types(T... args) -> decltype(std::tuple_cat(extract_type(args)...)) {
  return std::tuple_cat(extract_type(args)...);
}

template <typename T, int I>
using ntuple = typename repeat<T, I, std::tuple>::type;

template <typename dispatch_types_t,
          typename functor_type,
          typename return_type_t = typename functor_type::return_type,
          typename extra_params_t = std::tuple<> >
struct dispatch {
  using example_param_t = typename std::tuple_element<0, dispatch_types_t>::type;
  static constexpr auto num_params = std::tuple_size<example_param_t>::value;
  using dispatched_function_types = cat<ntuple<Base*, num_params>, extra_params_t>;
  using dispatch_types_base_t = cat<std::tuple<functor_type*>, dispatched_function_types>;
  using base_function_type_t = std::function<return_type_t(dispatch_types_base_t) >;
  using key_t = ntuple<std::type_index, num_params>;
  using func_map_t = std::unordered_map<key_t, base_function_type_t>;

  struct InitFunctor {
    func_map_t& _t;
    explicit InitFunctor(func_map_t& t) : _t(t) {}
    template <typename... T>
    void operator()(std::tuple<T...>&) {
      _t.insert(typename func_map_t::value_type(std::make_tuple(std::type_index(typeid(typename std::remove_pointer<T>::type))...),
                                                [] (dispatch_types_base_t params) {
                                                  using tn = cat<std::tuple<functor_type*, T...>, extra_params_t>;
                                                  static_assert(sizeof(tn) == sizeof(dispatch_types_base_t), "equal");
                                                  return apply<return_type_t>(call_special<return_type_t>, conv<tn>(params));
                                                }));
    }
  };

  func_map_t _choices;

  dispatch() {
    _choices.reserve(std::tuple_size<dispatch_types_t>::value);
    InitFunctor inserter(_choices);
    for_each(*(dispatch_types_t*) nullptr, inserter);
  }

  template <typename F, typename... ARGS>
  return_type_t call(F* functor, ARGS... args) {
    auto it = _choices.find(extract_types(args...));
    auto params = std::make_tuple(functor, args...);
    if (it != _choices.end()) {
      return it->second(params);
    } else {
      return apply<return_type_t>(call_special<return_type_t>, params);
    }
  }

  template <int num=0, bool done = num == num_params, typename... TS>
  struct check;

  template <int num, typename T, typename... TS>
  struct check<num, false, T, TS...> {
    static_assert(std::is_polymorphic<typename std::remove_pointer<T>::type>::value, "Param must be polymorphic");
    check<num+1, num+1 == num_params, TS...> c;
  };

  template <int num, typename... TS>
  struct check<num, true, TS...> {
    static_assert(num != 0, "dispatch on zero params");
  };

  template <typename... call_types_t>
  return_type_t operator()(functor_type& f, call_types_t... args) {
    check<0, 0 == num_params, call_types_t...>  __attribute__ ((unused)) c;
    static_assert(std::tuple_size<std::tuple<call_types_t...>>::value == std::tuple_size<dispatched_function_types>::value, "Trying to call with too few parameters");
    return call(&f, args...);
  }
};
