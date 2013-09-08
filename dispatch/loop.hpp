#pragma once

template <int I, int E, bool STOP = (I==E)>
struct loop;

template <int I, int E>
struct loop<I, E, true> {
  template <typename F>
  void operator()(F) {}
};

template <int I, int E, bool STOP>
struct loop {
  template<typename F>
  void operator()(F f) {
    f.template operator()<I>();
    loop<I+1, E>()(f);
  }
};
