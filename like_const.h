#pragma once

template <typename T>
struct NonConst {typedef T type;};
template <typename T>
struct NonConst<T const> {typedef T type;}; //by value
template <typename T>
struct NonConst<T const&> {typedef T& type;}; //by reference
template <typename T>
struct NonConst<T const*> {typedef T* type;}; //by pointer
template <typename T>
struct NonConst<T const&&> {typedef T&& type;}; //by rvalue-reference

template<typename TConstReturn, class TObj, typename... TArgs>
typename NonConst<TConstReturn>::type likeConstVersion(
    TObj const* obj,
    TConstReturn (TObj::* memFun)(TArgs...) const,
                                                          TArgs... args) {
  return const_cast<typename NonConst<TConstReturn>::type>(
      (obj->*memFun)(args...));
}
