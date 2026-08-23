#include "CsvReader.h"

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <stdexcept>
#include <utility>

namespace {
using FilePtr = std::unique_ptr<FILE, int (*)(FILE*)>;
}  // namespace

namespace CsvReader {

std::vector<GameRecord> readGames(const std::string& path) {
    FilePtr file(fopen(path.c_str(), "rb"), &fclose);
    if (!file) {
        throw std::runtime_error("Could not open CSV file: " + path);
    }

    // Read the whole file in one fread() call rather than std::getline()-ing
    // line by line -- one buffer fill instead of many small reads.
    fseek(file.get(), 0, SEEK_END);
    const long size = ftell(file.get());
    fseek(file.get(), 0, SEEK_SET);
    std::string buffer(static_cast<size_t>(size), '\0');
    fread(&buffer[0], 1, static_cast<size_t>(size), file.get());
    const char* data = buffer.c_str();

    // Cheap upfront pass to size the output vector once, avoiding the
    // amortized-doubling reallocation/copy churn of repeated push_back.
    const size_t estimatedRows =
        static_cast<size_t>(std::count(buffer.begin(), buffer.end(), '\n'));
    std::vector<GameRecord> games;
    games.reserve(estimatedRows);

    // Fields are parsed directly against pointers into `buffer` with strtoll/
    // strtod rather than cutting each line and field into its own std::string
    // first (which allocates ~9 small strings per row -- line + trailing-CR
    // trim + 7 fields -- for no benefit, since every field but season_year is
    // immediately converted to a number and thrown away). This version
    // allocates once per row, only for season_year.
    //
    // Note from profiling this on the local toolchain (MinGW GCC 6.3): after
    // this change, most of the remaining time is spent inside strtod/strtoll
    // themselves (~0.5us/call measured in isolation here), not in field
    // splitting or allocation -- an artifact of this specific, dated C
    // runtime's numeric parsing rather than something fixable by further
    // restructuring this loop.
    size_t lineStart = 0;
    bool isHeader = true;
    while (lineStart < buffer.size()) {
        size_t lineEnd = buffer.find('\n', lineStart);
        if (lineEnd == std::string::npos) {
            lineEnd = buffer.size();
        }
        size_t contentEnd = lineEnd;
        if (contentEnd > lineStart && data[contentEnd - 1] == '\r') {
            --contentEnd;
        }

        if (contentEnd == lineStart) {
            lineStart = lineEnd + 1;
            continue;
        }
        if (isHeader) {
            isHeader = false;
            lineStart = lineEnd + 1;
            continue;
        }

        // Expected column order: game_id,season_year,model_prob_home,implied_home,
        // decimal_home,decimal_away,home_win
        size_t pos = lineStart;
        auto nextField = [&](size_t limit) -> std::pair<size_t, size_t> {
            size_t comma = buffer.find(',', pos);
            if (comma == std::string::npos || comma > limit) {
                comma = limit;
            }
            const size_t start = pos;
            pos = comma + 1;
            return std::make_pair(start, comma);
        };

        char* endptr = nullptr;

        const std::pair<size_t, size_t> gameIdField = nextField(contentEnd);
        const long long gameId = std::strtoll(data + gameIdField.first, &endptr, 10);

        const std::pair<size_t, size_t> seasonField = nextField(contentEnd);
        std::string seasonYear = buffer.substr(seasonField.first, seasonField.second - seasonField.first);

        const std::pair<size_t, size_t> modelProbField = nextField(contentEnd);
        const double modelProbHome = std::strtod(data + modelProbField.first, &endptr);

        const std::pair<size_t, size_t> impliedField = nextField(contentEnd);
        const double impliedHome = std::strtod(data + impliedField.first, &endptr);

        const std::pair<size_t, size_t> decHomeField = nextField(contentEnd);
        const double decimalHome = std::strtod(data + decHomeField.first, &endptr);

        const std::pair<size_t, size_t> decAwayField = nextField(contentEnd);
        const double decimalAway = std::strtod(data + decAwayField.first, &endptr);

        const std::pair<size_t, size_t> winField = nextField(contentEnd);
        const bool homeWin = winField.second > winField.first && data[winField.first] != '0';

        GameRecord game;
        game.gameId = gameId;
        game.seasonYear = std::move(seasonYear);
        game.modelProbHome = modelProbHome;
        game.impliedHome = impliedHome;
        game.decimalHome = decimalHome;
        game.decimalAway = decimalAway;
        game.homeWin = homeWin;
        games.push_back(std::move(game));

        lineStart = lineEnd + 1;
    }

    return games;
}

}  // namespace CsvReader
