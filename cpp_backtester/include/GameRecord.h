#pragma once

#include <cstdint>
#include <string>

// One row of the backtest input: a single NBA game with the model's predicted
// home win probability, the market's (de-vigged) implied probability from the
// opening line, the real payout odds, and the actual outcome.
struct GameRecord {
    int64_t gameId = 0;
    std::string seasonYear;
    double modelProbHome = 0.0;
    double impliedHome = 0.0;
    double decimalHome = 0.0;
    double decimalAway = 0.0;
    bool homeWin = false;

    double edgeHome() const { return modelProbHome - impliedHome; }
};
