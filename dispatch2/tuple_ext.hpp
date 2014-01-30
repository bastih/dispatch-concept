/* Copyright (C) 2012-2013 Justin Berger 
   The full license is available in the LICENSE file at the root of this project and is also available at http://opensource.org/licenses/MIT. */
#pragma once
#include <tuple>
#include <map>
#include <stdexcept>
#include <vector>

namespace tupleExt {
  /* -------------------- iterate -------------------- */
  template <std::size_t N = 0, typename F, typename... Args>
    inline auto iterate(const F& , const std::tuple<Args...>&) -> typename std::enable_if<N == sizeof...(Args), void>::type { }

  template <std::size_t N = 0, typename F, typename... Args>
    inline auto iterate(F& f, const std::tuple<Args...>& t) -> typename std::enable_if<N < sizeof...(Args), void>::type{
    f( std::get<N>(t) );
    iterate<N+1, F, Args...>(f, t);
  }

  /** Exists so you can make the functor in place. */
  template <std::size_t N = 0, typename F, typename... Args>
    inline auto iterate(const F& _f, const std::tuple<Args...>& t) -> typename std::enable_if<N == 0, void>::type {
    F f(_f);
    iterate<N, F, Args...>(f, t);
  }
  /* -------------------- /iterate -------------------- */

  /* -------------------- iterate_i -------------------- */
  template <std::size_t N = 0, typename F, typename... Args>
    inline auto iterate_i(const F& , const std::tuple<Args...>&) -> typename std::enable_if<N == sizeof...(Args), void>::type { }

  template <std::size_t N = 0, typename F, typename... Args>
    inline auto iterate_i(F& f, const std::tuple<Args...>& t) -> typename std::enable_if<N < sizeof...(Args), void>::type{
    f(N,  std::get<N>(t) );
    iterate_i<N+1, F, Args...>(f, t);
  }
  
  /** Exists so you can make the functor in place. */
  template <std::size_t N = 0, typename F, typename... Args>
    inline auto iterate_i(const F& _f, const std::tuple<Args...>& t) -> typename std::enable_if<N == 0, void>::type {
    F f(_f);
    iterate_i<N, F, Args...>(f, t);
  }
  /* -------------------- /iterate_i -------------------- */

  /* -------------------- make_jumptable -------------------- */
  template <typename F, typename FT, std::size_t N, typename... Args>
    auto inline make_jumptable(FT* jump[sizeof...(Args)]) -> typename std::enable_if< N == sizeof...(Args), void>::type {
  }
 
  template <typename F, typename FT, std::size_t N, typename... Args>
    auto inline make_jumptable(FT* jump[sizeof...(Args)]) 
    -> typename std::enable_if< N < sizeof...(Args), void>::type {
    jump[N] = [](F& f, const std::tuple<Args...>& t) { f( std::get<N>(t)); };  
    make_jumptable<F, FT, N+1, Args...>(jump);
  }

  template <typename F, typename FT, std::size_t N, typename... Args>
    auto inline make_jumptable_rtn(FT* jump[sizeof...(Args)]) 
    -> typename std::enable_if< N == sizeof...(Args), void>::type {
  }

  template <typename F, typename FT, std::size_t N, typename... Args>
    auto inline make_jumptable_rtn(FT* jump[sizeof...(Args)]) 
    -> typename std::enable_if< N < sizeof...(Args), void>::type {
    jump[N] = [](F& f, const std::tuple<Args...>& t) { return f( std::get<N>(t)); };  
    make_jumptable_rtn<F, FT, N+1, Args...>(jump);
  }

  /* -------------------- /make_jumptable -------------------- */

  /* -------------------- run -------------------- */
  template <typename F, typename... Args>
    auto inline run( F& f, const size_t i, const std::tuple<Args...>& t) -> void
  {
    using FT = void ( F&, const std::tuple<Args...>& );
    static FT* jump[sizeof...(Args)];
    static bool init = false;
    if(!init){    
      init = true;
      make_jumptable<F, FT, 0, Args...>(jump);
    }
    jump[i](f, t);
  }
  /* -------------------- /run -------------------- */

  /* -------------------- get -------------------- */
  template <typename F, typename... Args>
    auto inline get( F& f, const size_t i, const std::tuple<Args...>& t) -> decltype(f(std::get<0>(t) ))
  {
    using FT = decltype(f( std::get<0>(t))) ( F&, const std::tuple<Args...>& );
    static FT* jump[sizeof...(Args)];
    static bool init = false;
    if(!init){    
      init = true;
      make_jumptable_rtn<F, FT, 0, Args...>(jump);
    }
    return jump[i](f, t);
  }
  /* -------------------- /get -------------------- */

  /* -------------------- to_vector -------------------- */
  template <std::size_t N = 0, typename F, typename Rtn, typename... Args>
    static inline auto to_vector(const F& , const std::tuple<Args...>&, std::vector<Rtn>& ) 
    -> typename std::enable_if<N == sizeof...(Args), void>::type { }

  template <std::size_t N = 0, typename F, typename Rtn, typename... Args>
    static inline auto to_vector(F& f, const std::tuple<Args...>& t, std::vector<Rtn>& vector) 
    -> typename std::enable_if<N < sizeof...(Args), void>::type
    {
      vector[N] = f(std::get<N>(t) );
      to_vector<N+1, F, Rtn, Args...>(f, t, vector);
    }
  
  template <typename F, typename... Args>
    static inline auto to_vector(F& f, const std::tuple<Args...>& t) 
    -> std::vector< decltype(f(std::get<0>(t) )) > 
  {
    using Rtn = decltype(f(std::get<0>(t) ));
    std::vector<Rtn> vector;
    vector.resize(sizeof...(Args));
    to_vector<0, F, Rtn, Args...>(f, t, vector);
    return vector;
  }

  /** Exists so you can make the functor in place. */
  template <typename F, typename Rtn, typename... Args>
    static inline auto to_vector(const F& _f, const std::tuple<Args...>& t) 
    -> std::vector< decltype(f(std::get<0>(t) )) > 
  {
    F f(_f);
    return to_vector<F, Rtn, Args...>(f, t);
  }
  /* -------------------- /to_vector -------------------- */
  /* -------------------- fold -------------------- */
  template <std::size_t N = 0, typename F, typename Fold, typename... Args, class = typename std::enable_if<N == sizeof...(Args), void>::type>
  inline decltype(auto) fold(const F& , const std::tuple<Args...>, const Fold fold)
 { 
    return fold;
  }

  template <std::size_t N = 0, typename F, typename Fold, typename... Args, class = typename std::enable_if<N < sizeof...(Args), void>::type  >
            inline decltype(auto) fold(F& f, const std::tuple<Args...> t, Fold fold_v) {
    auto fold_new = f(fold_v,  std::get<N>(t) );
    return fold<N+1, F, decltype(fold_new), Args...>(f, t, fold_new);
  }
  /*
  
    template <std::size_t N = 0, typename F, typename Fold, typename... Args, class = typename std::enable_if<N < sizeof...(Args), void>::type> 
              inline decltype(auto) fold(const F& _f, const std::tuple<Args...>& t, const Fold& _fold)  {
    F f(_f);
    Fold fold_v(_fold);
    return fold<N, F, Fold, Args...>(f, t, fold_v);
    } */

  /* -------------------- /fold -------------------- */

  /* -------------------- MappedTuple_ -------------------- */
  template <typename K, typename KC, typename... T>
    struct MappedTuple_ : public std::tuple<T...> {
    std::map<K, int> map;
    KC kc;
     
    template <typename Tn>
      void operator()(unsigned int idx, Tn& t){
      map[ kc(t) ] = idx; 
    }
     
  MappedTuple_(KC f, T... t) : kc(f), std::tuple<T...>(t...) {
      iterate_i(*this, *this);
    }
 
  MappedTuple_(T... t) : std::tuple<T...>(t...) {
      iterate_i(*this, *this);
    }

    template <typename E>
      void operator()(const K& key, E& e){
      run( map[key], e, *this);
    }
     
    template <typename E>
      auto findAndRun(const K& key, const E& _e) -> void {
      E e(_e);
      run( map[key], e, *this);
    }

  };
  /* -------------------- /MappedTuple_ -------------------- */

  /** 
      The original concept for the tuple functionality below was largely found at http://stackoverflow.com/a/1547118. 
      
      It recursively unrolls a tuple, and applies it to function. 
      So basically if you have:
      
      tuple<Arg1, Arg2, Arg3, Arg4> tuple;

      applyTuple(t, &T::f, tuple)
      is equivalent to
      t->f( get<0>(tuple), get<1>(tuple), get<2>(tuple), get<3>(tuple));

      There are a few variations below, namely for return types and static functions. This should all probably 
      be part of the standard. 

      The modifications from the original posting were to change all the types to reference types, and add some overloads. 
  */
  template < uint N >
    struct apply {
      template < typename T, typename... ArgsF, typename... ArgsT, typename... Args >
	static inline void applyTuple( T* pObj,
				       void (T::*f)( ArgsF... ),
				       std::tuple<ArgsT...>& t,
				       Args&... args ) {
	apply<N-1>::applyTuple( pObj, f, t, std::get<N-1>( t ), args... );
      }

      template < typename T, typename... ArgsF, typename... ArgsT, typename... Args >
	static inline void applyTuple_obj(T* pObj, 
					  void (T::*f)( ArgsF... ),
					  std::tuple<ArgsT...>& t,
					  Args&... args ) {
	apply<N-1>::applyTuple_obj( pObj, f, t, std::get<N-1>( t ), args... );
      }

      template < typename T, typename retF, typename... ArgsF, typename... ArgsT, typename... Args >
	static inline retF applyTuple_obj_ret(T* pObj, 
					      retF (T::*f)( ArgsF... ),
					      std::tuple<ArgsT...>& t,
					      Args&... args ) {
	return apply<N-1>::applyTuple_obj_ret( pObj, f, t, std::get<N-1>( t ), args... );
      }

    };

  /**
     This is the specialization of apply which marks the termination of the recursive expansion.
     \sa apply
  */
  template <>
    struct apply<0> {
    template < typename T, typename... ArgsF, typename... ArgsT, typename... Args >
      static inline void applyTuple_obj( T* pObj,
					 void (T::*f)( ArgsF... ),
					 std::tuple<ArgsT...>& /* t */,
					 Args&... args )
    {
      (pObj->*f)( args... );
    }

    template < typename T, typename retF, typename... ArgsF, typename... ArgsT, typename... Args >
      static inline retF applyTuple_obj_ret( T* pObj,
					     retF (T::*f)( ArgsF... ),
					     std::tuple<ArgsT...>& /* t */,
					     Args&... args )
    {
      return (pObj->*f)( args... );
    }
  };

  template < typename... ArgsF, typename... ArgsT >
    static inline void applyTuple( void (*f)(ArgsF...),
				   std::tuple<ArgsT...>& t )
  {
    apply<sizeof...(ArgsT)>::applyTuple( f, t );
  }

  template < typename T, typename... ArgsF, typename... ArgsT >
    static inline void applyTuple( T* pObj, void (T::*f)(ArgsF...), std::tuple<ArgsT...>& t )
  {
    apply<sizeof...(ArgsT)>::applyTuple_obj(pObj, f, t );
  }

  template < typename T, typename retF, typename... ArgsF, typename... ArgsT >
    static inline retF applyTuple( T* pObj, retF (T::*f)(ArgsF...),
				   std::tuple<ArgsT...>& t )
  {
    return apply<sizeof...(ArgsT)>::applyTuple_obj_ret(pObj, f, t );
  }

}
