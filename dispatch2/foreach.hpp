#pragma once
#include <tuple>

// ------------- UTILITY---------------
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

// ----------- FOR EACH -----------------
template<typename Func, typename Last>
void for_each_impl(Func&& f, Last&& last)
{
  f(last);
}

template<typename Func, typename First, typename ... Rest>
void for_each_impl(Func&& f, First&& first, Rest&&...rest)
{
  f(first);
  for_each_impl( std::forward<Func>(f), rest...);
}

template<typename Func, int ... Indexes, typename ... Args>
void for_each_helper( Func&& f, index_tuple<Indexes...>, std::tuple<Args...>&& tup)
{
  for_each_impl( std::forward<Func>(f), std::forward<Args>(std::get<Indexes>(tup))...);
}

template<typename Func, typename ... Args>
void for_each( std::tuple<Args...>& tup, Func&& f)
{
  for_each_helper(std::forward<Func>(f),
                  typename make_indexes<Args...>::type(),
                  std::forward<std::tuple<Args...>>(tup) );
}

template<typename Func, typename ... Args>
void for_each( std::tuple<Args...>&& tup, Func&& f)
{
  for_each_helper(std::forward<Func>(f),
                  typename make_indexes<Args...>::type(),
                  std::forward<std::tuple<Args...>>(tup) );
}



template <class Ret, class... Args, int... Indexes>
Ret apply_helper(Ret (*pf)(Args...), index_tuple<Indexes...>, /*std::tuple<Rep...> t,*/ std::tuple<Args...>&& tup) {
  return pf(std::get<Indexes>(tup)...);
}

template <class Ret, class... Args>
Ret apply(Ret (*pf)(Args...), const std::tuple<Args...>& tup /*std::tuple<Rep...> t*/) {
  return apply_helper<Ret>(pf, typename make_indexes<Args...>::type(), /*t,*/ std::tuple<Args...>(tup));
}

template <class Ret, class... Args>
Ret apply(Ret (*pf)(Args...), std::tuple<Args...>&& tup) {
  return apply_helper(pf, typename make_indexes<Args...>::type(), std::forward<std::tuple<Args...>>(tup));
}


template <class... Args, class... CastArgs, int... Indexes>
std::tuple<CastArgs...> conv_helper(const std::tuple<Args...>& tuple, index_tuple<Indexes...>, std::tuple<CastArgs...>) {
  return std::make_tuple(static_cast<typename std::tuple_element<Indexes, std::tuple<CastArgs...>>::type>(std::get<Indexes>(tuple))...);
}

template <class CastArg, class... Args>
CastArg conv(const std::tuple<Args...>& source) {
  return conv_helper(source, typename make_indexes<Args...>::type(), CastArg());
}
