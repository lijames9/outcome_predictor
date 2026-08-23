#pragma once

#include <string>
#include <vector>

#include "GameRecord.h"

// Reads backtest_input.csv into a vector of GameRecord.
//
// Performance notes (this is the point of writing it in C++ rather than using
// a library): the whole file is read into memory in one syscall rather than
// line-by-line, and each line is parsed with manual index scanning (find/substr
// on positions) instead of std::istringstream, which is well known to be slow
// due to per-token stream state overhead. The output vector is reserve()'d
// once the row count is known so there are no reallocation copies while filling
// it in -- important for CSVs of this shape where the row count can be in the
// hundreds of thousands for a full multi-decade dataset.
namespace CsvReader {

// Throws std::runtime_error if the file can't be opened or a row is malformed.
std::vector<GameRecord> readGames(const std::string& path);

}  // namespace CsvReader
