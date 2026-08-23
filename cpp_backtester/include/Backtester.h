#pragma once

#include <string>
#include <vector>

#include "GameRecord.h"

// Result of running the backtest at one edge threshold.
struct BacktestResult {
    double threshold = 0.0;
    int nBets = 0;
    double winRate = 0.0;  // NaN if nBets == 0
    double roi = 0.0;      // NaN if nBets == 0, else total profit / nBets (flat 1-unit stake)
};

namespace Backtester {

// Mirrors the Python edge-detection logic exactly:
//   edge = model_prob_home - implied_home
//   bet the side (home/away) the model favors more than the market, for games
//   where |edge| >= threshold; flat 1-unit stake; profit = decimalOdds - 1 on
//   a win, -1 on a loss.
std::vector<BacktestResult> runSweep(const std::vector<GameRecord>& games,
                                      const std::vector<double>& thresholds);

// Splits games by season_year: everything in `holdoutSeason` goes to .second,
// everything else to .first.
std::pair<std::vector<GameRecord>, std::vector<GameRecord>> splitByHoldoutSeason(
    const std::vector<GameRecord>& games, const std::string& holdoutSeason);

}  // namespace Backtester
