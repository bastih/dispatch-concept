#pragma once
#include "storage.h"
#include "make_unique.h"

constexpr dis_int UPPER_VID = 1000;
constexpr value_id_t DEFAULT_VID = 10;

std::unique_ptr<ATable> makeStore();
