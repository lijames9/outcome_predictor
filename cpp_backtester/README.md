# NBA Betting Backtester (C++)

A standalone C++ port of the edge-detection backtest from the [NBA Betting project](../data_analysis.ipynb)'s
Python/pandas analysis. Reads the model's predicted win probabilities alongside the market's
odds, and runs the same threshold-sweep ROI backtest, split into an exploration period and a
held-out season, entirely in C++.

## Why this exists

The core analysis (data cleaning, feature engineering, model training) lives in the Jupyter
notebook -- that's the right tool for iterative, exploratory data work. This backtester takes
the *output* of that analysis (predicted probabilities + market odds + real outcomes, one row
per game) and re-implements just the final evaluation step in C++, as a small, focused
demonstration of:

- **Fast, allocation-conscious CSV parsing**: the file is read into memory in a single read()
  call rather than line-by-line, row count is precomputed with a single pass so the output
  `std::vector` is sized once via `reserve()` (avoiding repeated reallocation/copy as it grows),
  and each line is split with plain index scanning instead of `std::istringstream` (which is
  well known to be slow for this due to per-token stream state overhead).
- **Clean separation of I/O, domain logic, and orchestration**: `CsvReader` only knows how to
  turn a file into `GameRecord`s; `Backtester` only knows how to score a set of `GameRecord`s
  against a threshold; `main` just wires the two together and prints results.

## Building

Requires a C++14 compiler (developed/tested against `g++` 6.3, so it deliberately avoids C++17
features like structured bindings and `std::string_view`).

With `make`:

```
make
./backtester data/backtest_input.csv
```

Without `make` (e.g. plain MinGW g++):

```
g++ -std=c++14 -O2 -Iinclude -c src/main.cpp -o src/main.o
g++ -std=c++14 -O2 -Iinclude -c src/CsvReader.cpp -o src/CsvReader.o
g++ -std=c++14 -O2 -Iinclude -c src/Backtester.cpp -o src/Backtester.o
g++ -std=c++14 -o backtester src/main.o src/CsvReader.o src/Backtester.o
./backtester data/backtest_input.csv
```

## Input format

`data/backtest_input.csv` has one row per game, exported from the Python analysis's `test_df`:

```
game_id,season_year,model_prob_home,implied_home,decimal_home,decimal_away,home_win
```

## Result (for the record)

Consistent with the Python findings: no threshold produces a positive ROI on the held-out
2025-26 season, despite an encouraging-looking trend in the exploration seasons -- a clean
demonstration of why out-of-sample validation matters before trusting a backtest.
