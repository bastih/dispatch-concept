#pragma once

#define ALL(var) std::begin(var), std::end(var)
#define _AUTO(var) decltype(*std::begin(var))
#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)
