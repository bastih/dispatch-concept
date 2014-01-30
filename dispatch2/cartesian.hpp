#pragma once
template <typename S, typename T> struct mtuple_cat;

template <typename ...Ss, typename ...Ts>
struct mtuple_cat<std::tuple<Ss...>, std::tuple<Ts...>>
{
  typedef std::tuple<Ss..., Ts...> type;
};

template <typename S, typename T> struct product;

template <typename S, typename ...Ss, typename ...Ts>
struct product<std::tuple<S, Ss...>, std::tuple<Ts...>>
{
  // the cartesian product of {S} and {Ts...}
  // is a list of pairs -- here: a std::tuple of 2-element std::tuples
  typedef std::tuple<std::tuple<S, Ts>...> S_cross_Ts;
  
  // the cartesian product of {Ss...} and {Ts...} (computed recursively)
  typedef typename product<std::tuple<Ss...>, std::tuple<Ts...>>::type Ss_cross_Ts;
  
  // concatenate both products
  typedef typename mtuple_cat<S_cross_Ts, Ss_cross_Ts>::type type;
};

// end the recursion
template <typename ...Ts>
struct product<std::tuple<>, std::tuple<Ts...>>
{
  typedef std::tuple<> type;
};

template <typename S> struct flatten;

template <typename T, typename ... Ts>
struct flatten< std::tuple<T, Ts...> > {
  using type = typename mtuple_cat<
                      typename flatten<T>::type,
                      typename flatten<std::tuple<Ts...>>::type
                >::type;
};

template <typename T>
struct flatten {
  using type = std::tuple<T>;
};

template <>
struct flatten< std::tuple<> > {
  using type = std::tuple<>;
};

template <typename ...T> struct flat_typelists;

template <typename ...T>
struct flat_typelists< std::tuple<T...> > {
  using type = std::tuple< typename flatten<T>::type... >;
};

