#pragma once

#include "helpers/make_unique.h"
#include "storage/types_fwd.h"

constexpr dis_int UPPER_VID = 1000;
constexpr value_id_t DEFAULT_VID = 10;
class ATable;
std::unique_ptr<ATable> makeStore();
