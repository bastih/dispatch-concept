#pragma once

#include "helpers/make_unique.h"
#include "storage/types_fwd.h"

constexpr dis_int UPPER_VID = 1000;
constexpr value_id_t DEFAULT_VID = 10;
constexpr size_t MAINSIZE = 6 * 1000 * 1000;
constexpr size_t DELTASIZE = 2 * 1000 * 1000;

std::unique_ptr<ATable> makeEqualPartitionTable(std::size_t rows, std::size_t cols, std::size_t parts);
std::unique_ptr<ATable> makeCEqualPartitionTable(std::size_t rows, std::size_t cols, std::size_t parts);
std::unique_ptr<ATable> makeStore(std::size_t main_size=MAINSIZE, std::size_t delta_size=DELTASIZE);
std::unique_ptr<ATable> makeSmallStore();

std::unique_ptr<ATable> makeSomeTable();
