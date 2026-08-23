#include "Backtester.h"

#include <cmath>

namespace Backtester {

std::vector<BacktestResult> runSweep(const std::vector<GameRecord>& games,
                                      const std::vector<double>& thresholds) {
    std::vector<BacktestResult> results;
    results.reserve(thresholds.size());

    for (double threshold : thresholds) {
        int nBets = 0;
        int nWins = 0;
        double totalProfit = 0.0;

        for (const GameRecord& game : games) {
            const double edge = game.edgeHome();
            if (std::fabs(edge) < threshold) {
                continue;
            }

            const bool betHome = edge > 0.0;
            const double odds = betHome ? game.decimalHome : game.decimalAway;
            const bool won = betHome ? game.homeWin : !game.homeWin;

            ++nBets;
            if (won) {
                ++nWins;
                totalProfit += (odds - 1.0);
            } else {
                totalProfit -= 1.0;
            }
        }

        BacktestResult result;
        result.threshold = threshold;
        result.nBets = nBets;
        if (nBets > 0) {
            result.winRate = static_cast<double>(nWins) / nBets;
            result.roi = totalProfit / nBets;
        } else {
            result.winRate = std::nan("");
            result.roi = std::nan("");
        }
        results.push_back(result);
    }

    return results;
}

std::pair<std::vector<GameRecord>, std::vector<GameRecord>> splitByHoldoutSeason(
    const std::vector<GameRecord>& games, const std::string& holdoutSeason) {
    std::vector<GameRecord> exploration;
    std::vector<GameRecord> holdout;
    exploration.reserve(games.size());

    for (const GameRecord& game : games) {
        if (game.seasonYear == holdoutSeason) {
            holdout.push_back(game);
        } else {
            exploration.push_back(game);
        }
    }
    return {exploration, holdout};
}

}  // namespace Backtester
