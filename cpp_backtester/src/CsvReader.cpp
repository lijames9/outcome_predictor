#include "CsvReader.h"

#include <algorithm>
#include <fstream>
#include <stdexcept>

namespace {

// Splits one CSV line into its 7 fields by scanning for commas with plain
// index arithmetic -- no stringstream, no regex, no per-field heap churn
// beyond the substr calls the fields themselves need.
std::vector<std::string> splitLine(const std::string& line) {
    std::vector<std::string> fields;
    fields.reserve(7);
    size_t start = 0;
    while (true) {
        size_t comma = line.find(',', start);
        if (comma == std::string::npos) {
            fields.push_back(line.substr(start));
            break;
        }
        fields.push_back(line.substr(start, comma - start));
        start = comma + 1;
    }
    return fields;
}

std::string trimCr(const std::string& s) {
    if (!s.empty() && s.back() == '\r') {
        return s.substr(0, s.size() - 1);
    }
    return s;
}

}  // namespace

namespace CsvReader {

std::vector<GameRecord> readGames(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Could not open CSV file: " + path);
    }

    // Read the whole file in one shot rather than std::getline()-ing directly
    // from the stream -- one syscall/buffer fill instead of many small reads.
    file.seekg(0, std::ios::end);
    const std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);
    std::string buffer(static_cast<size_t>(size), '\0');
    file.read(&buffer[0], size);

    // Cheap upfront pass to size the output vector once, avoiding the
    // amortized-doubling reallocation/copy churn of repeated push_back.
    const size_t estimatedRows =
        static_cast<size_t>(std::count(buffer.begin(), buffer.end(), '\n'));
    std::vector<GameRecord> games;
    games.reserve(estimatedRows);

    size_t lineStart = 0;
    bool isHeader = true;
    while (lineStart < buffer.size()) {
        size_t lineEnd = buffer.find('\n', lineStart);
        if (lineEnd == std::string::npos) {
            lineEnd = buffer.size();
        }
        std::string line = trimCr(buffer.substr(lineStart, lineEnd - lineStart));
        lineStart = lineEnd + 1;

        if (line.empty()) {
            continue;
        }
        if (isHeader) {
            isHeader = false;
            continue;
        }

        // Expected column order: game_id,season_year,model_prob_home,implied_home,
        // decimal_home,decimal_away,home_win
        std::vector<std::string> fields = splitLine(line);
        if (fields.size() != 7) {
            throw std::runtime_error("Malformed CSV row (expected 7 fields, got " +
                                      std::to_string(fields.size()) + "): " + line);
        }

        GameRecord game;
        game.gameId = std::stoll(fields[0]);
        game.seasonYear = fields[1];
        game.modelProbHome = std::stod(fields[2]);
        game.impliedHome = std::stod(fields[3]);
        game.decimalHome = std::stod(fields[4]);
        game.decimalAway = std::stod(fields[5]);
        game.homeWin = std::stoi(fields[6]) != 0;
        games.push_back(std::move(game));
    }

    return games;
}

}  // namespace CsvReader
