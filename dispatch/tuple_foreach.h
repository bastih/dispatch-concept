#pragma once

#include <tuple>


namespace detail {

template <typename... Args>
struct Impl;

template <typename First, typename... Args>
struct Impl<First, Args...> { static std::string name() { return std::string(typeid(First).name()) + " " + Impl<Args...>::name(); } };

template <>
struct Impl<> { static std::string name() { return ""; } };

}

template <typename... Args>
std::string type_names() {  return detail::Impl<Args...>::name(); }


template <int...>
struct index_tuple {};

template <int I, typename IndexTuple, typename... Types>
struct make_indexes_impl;

template <int I, int... Indexes, typename T, typename... Types>
struct make_indexes_impl<I, index_tuple<Indexes...>, T, Types...> {
  typedef typename make_indexes_impl<I + 1, index_tuple<Indexes..., I>,
                                     Types...>::type type;
};

template <int I, int... Indexes>
struct make_indexes_impl<I, index_tuple<Indexes...>> {
  typedef index_tuple<Indexes...> type;
};

template <typename... Types>
struct make_indexes : make_indexes_impl<0, index_tuple<>, Types...> {};



template <class Ret, class... Args, int... Indexes>
Ret apply_helper(Ret (*pf)(Args...), index_tuple<Indexes...>, std::tuple<Args...>&& tup) {
  return pf(std::forward<Args>(std::get<Indexes>(tup))...);
}

template <class Ret, class... Args>
Ret apply(Ret (*pf)(Args...), const std::tuple<Args...>& tup) {
  return apply_helper(pf, typename make_indexes<Args...>::type(), std::tuple<Args...>(tup));
}

template <class Ret, class... Args>
Ret apply(Ret (*pf)(Args...), std::tuple<Args...>&& tup) {
  return apply_helper(pf, typename make_indexes<Args...>::type(), std::forward<std::tuple<Args...>>(tup));
}

template< std::size_t... Ns >
struct indices
{
  typedef indices< Ns..., sizeof...( Ns ) > next;
};

template< std::size_t N >
struct make_indices
{
  typedef typename make_indices< N - 1 >::type::next type;
};

template<>
struct make_indices< 0 >
{
  typedef indices<> type;
};

template< typename Tuple, std::size_t N, typename T,
          typename Indices = typename make_indices< std::tuple_size< Tuple >::value >::type >
struct element_replace;

template< typename... Ts, std::size_t N, typename T, std::size_t... Ns >
struct element_replace< std::tuple< Ts... >, N, T, indices< Ns... > >
{
  typedef std::tuple< typename std::conditional< Ns == N, T, Ts >::type... > type;
};

template< typename Tuple, std::size_t N, 
          typename Indices = typename make_indices< std::tuple_size< Tuple >::value >::type >
struct split;

template< typename... Ts, std::size_t N, std::size_t ... Ns >
struct split < std::tuple < Ts... >, N, indices < Ns... > > {
using head = decltype(std::tuple_cat(typename std::conditional< Ns < N, std::tuple<Ts>, std::tuple<> >::type()...));
using tail = decltype(std::tuple_cat(typename std::conditional< Ns < N, std::tuple<>, std::tuple<Ts> >::type()...));
};

template <int I, class Tuple, typename F, int UNTIL, bool STOP = I==UNTIL>
struct for_each_impl;

template <int I, class Tuple, typename F, int UNTIL, bool STOP>
struct for_each_impl {
  static void for_each(const Tuple& t, F f) {
    for_each_impl<I - 1, Tuple, F, UNTIL>::for_each(t, f);
    f(std::get<I>(t));
  }
};

template <int I, class Tuple, typename F, int UNTIL>
struct for_each_impl<I, Tuple, F, UNTIL, true> {
  static void for_each(const Tuple& t, F f) { f(std::get<I>(t)); }
};

template <class Tuple, typename F>
F for_each(const Tuple& t, F f) {
  for_each_impl<std::tuple_size<Tuple>::value - 1, Tuple, F, 0>::for_each(t, f);
  return f;
}


template <int COUNT, class Tuple, typename F>
F for_each_c(const Tuple& t, F f) {
for_each_impl<COUNT-1, Tuple, F, 0>::for_each(t, f);
return f;
}
