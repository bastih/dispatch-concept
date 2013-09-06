template <size_t... n>
struct ct_integers_list {
  template <size_t m>
  struct push_back {
    typedef ct_integers_list<n..., m> type;
  };
};

template <size_t max>
struct ct_iota_1 {
  typedef typename ct_iota_1<max - 1>::type::template push_back<max>::type type;
};

template <>
struct ct_iota_1<0> {
  typedef ct_integers_list<> type;
};

template <size_t... indices, typename Tuple>
auto tuple_subset(const Tuple& tpl, ct_integers_list<indices...>)
    -> decltype(std::make_tuple(std::get<indices>(tpl)...)) {
  return std::make_tuple(std::get<indices>(tpl)...);
  // this means:
  //   make_tuple(get<indices[0]>(tpl), get<indices[1]>(tpl), ...)
}

template <typename Head, typename... Tail>
std::tuple<Tail...> tuple_tail(const std::tuple<Head, Tail...>& tpl) {
  return tuple_subset(tpl, typename ct_iota_1<sizeof...(Tail)>::type());
  // this means:
  //   tuple_subset<1, 2, 3, ..., sizeof...(Tail)-1>(tpl, ..)
}

template <typename Head, typename... Tail, int N>
auto tuple_tail_n(const std::tuple<Head, Tail...>& tpl) -> decltype(tuple_subset(tpl, typename ct_iota_1<N>::type())) {
  return tuple_subset(tpl, typename ct_iota_1<N>::type());
}


template <int I, class Tuple, typename F>
struct for_each_impl {
  static void for_each(const Tuple& t, F f) {
    for_each_impl<I - 1, Tuple, F>::for_each(t, f);
    f(std::get<I>(t));
  }
};
template <class Tuple, typename F>
struct for_each_impl<0, Tuple, F> {
  static void for_each(const Tuple& t, F f) { f(std::get<0>(t)); }
};
template <class Tuple, typename F>
F for_each(const Tuple& t, F f) {
  for_each_impl<std::tuple_size<Tuple>::value - 1, Tuple, F>::for_each(t, f);
  return f;
}
