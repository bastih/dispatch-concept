#pragma once

template<typename, typename>
struct append_to_type_seq { };

template<typename T, typename... Ts, template<typename...> class TT>
struct append_to_type_seq<T, TT<Ts...>> {
  using type = TT<Ts..., T>;
};

template<typename T, unsigned int N, template<typename...> class TT>
struct repeat {
  using type = typename append_to_type_seq<T, typename repeat<T, N-1, TT>::type >::type;
};

template<typename T, template<typename...> class TT>
struct repeat<T, 0, TT>
{
  using type = TT<>;
};
