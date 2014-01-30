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

template <typename T>
typename T::mapped_type mget(const T& map, typename T::key_type key, typename T::mapped_type df) {
  auto it = map.find(key);
  return it != map.end() ? it->second : df;
}

template < class RETURN_TYPE, class O, class... ARGS>
RETURN_TYPE call_special(O* op, ARGS... args) {
  return op->execute(args...);
}

template <typename T, int I>
using ntuple = typename repeat<T, I, std::tuple>::type;

template <typename dispatch_types_t, typename functor_type, typename return_type_t = typename functor_type::return_type>
struct dispatch {
  using example_param_t = typename std::tuple_element<0, dispatch_types_t>::type;
  static constexpr auto num_params = std::tuple_size<example_param_t>::value;
  using dispatch_types_base_t = typename mtuple_cat<std::tuple<functor_type*>, ntuple<Base*, num_params>>::type; 
  using base_function_type_t = std::function<return_type_t(dispatch_types_base_t) >;
  using key_t = ntuple<std::type_index, num_params>;
  using func_map_t = std::unordered_map<key_t, base_function_type_t>;
  
  struct InitFunctor {
    func_map_t& _t;
    explicit InitFunctor(func_map_t& t) : _t(t) {}
    template<typename... T>
    void operator()(std::tuple<T...>&) {
      _t.insert({std::make_tuple(std::type_index(typeid(typename std::remove_pointer<T>::type))...),
            [] (dispatch_types_base_t params) {
            using tn = std::tuple<functor_type*, T...>;
            return apply<return_type_t>(call_special<return_type_t>, params, tn());
          }
        });
    }
  };

  func_map_t _choices;
  base_function_type_t _default;

  dispatch() {
    InitFunctor inserter(_choices);
    for_each(dispatch_types_t(), inserter);
    _default = [&] (dispatch_types_base_t params) -> return_type_t { 
      return apply<return_type_t>(call_special<return_type_t>, params, params); 
    };
  }

  template <typename F, typename... ARGS>
  return_type_t call(F* functor, ARGS... args) {
    return mget(_choices, 
                std::make_tuple(std::type_index(typeid(*args))...), 
                _default)(std::make_tuple(functor, args...));
  }
  
  template <typename... call_types_t>
  return_type_t operator()(functor_type& f, call_types_t... args) {
    return call(&f, args...);
  }
};
