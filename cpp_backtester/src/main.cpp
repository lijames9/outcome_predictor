#include <chrono>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

#include "Backtester.h"
#include "CsvReader.h"
#include "GameRecord.h"

namespace {

void printTable(const std::string& title, const std::vector<BacktestResult>& results) {
    std::cout << "\n" << title << "\n";
    std::cout << std::left << std::setw(11) << "threshold" << std::right << std::setw(10) << "n_bets"
              << std::setw(12) << "win_rate" << std::setw(12) << "roi" << "\n";
    for (const BacktestResult& r : results) {
        std::cout << std::left << std::setw(11) << std::fixed << std::setprecision(2) << r.threshold
                   << std::right << std::setw(10) << r.nBets << std::setw(12) << std::setprecision(6)
                   << r.winRate << std::setw(12) << r.roi << "\n";
    }
}

}  // namespace

int main(int argc, char** argv) {
    const std::string csvPath =
        (argc > 1) ? argv[1] : "data/backtest_input.csv";
    const std::string holdoutSeason = "2025-26";
    const std::vector<double> thresholds = {0.02, 0.04, 0.06, 0.08, 0.10, 0.12, 0.15, 0.20};

    std::vector<GameRecord> games;
    try {
        const auto readStart = std::chrono::steady_clock::now();
        games = CsvReader::readGames(csvPath);
        const auto readEnd = std::chrono::steady_clock::now();
        const double readMs =
            std::chrono::duration<double, std::milli>(readEnd - readStart).count();

        std::cout << "Loaded " << games.size() << " games from " << csvPath << " in " << std::fixed
                  << std::setprecision(3) << readMs << " ms\n";
    } catch (const std::exception& e) {
        std::cerr << "Error reading CSV: " << e.what() << "\n";
        return 1;
    }

    const auto computeStart = std::chrono::steady_clock::now();

    const auto splitGames = Backtester::splitByHoldoutSeason(games, holdoutSeason);
    const std::vector<GameRecord>& exploration = splitGames.first;
    const std::vector<GameRecord>& holdout = splitGames.second;
    std::cout << "Exploration: " << exploration.size() << " games (everything before " << holdoutSeason
              << ")\n";
    std::cout << "Holdout:     " << holdout.size() << " games (" << holdoutSeason << ")\n";

    const std::vector<BacktestResult> explorationResults = Backtester::runSweep(exploration, thresholds);
    const std::vector<BacktestResult> holdoutResults = Backtester::runSweep(holdout, thresholds);

    const auto computeEnd = std::chrono::steady_clock::now();
    const double computeMs =
        std::chrono::duration<double, std::milli>(computeEnd - computeStart).count();

    printTable("Exploration set -- extended threshold sweep:", explorationResults);
    printTable("Holdout set (" + holdoutSeason + ", checked once):", holdoutResults);

    std::cout << "\nBacktest computation (both sweeps): " << std::fixed << std::setprecision(3)
              << computeMs << " ms\n";

    return 0;
}
