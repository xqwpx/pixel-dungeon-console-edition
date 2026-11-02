#pragma once

#include <string>
#include <vector>

#include "game/Progression.h"

namespace pdv {

struct OnlineLeaderboardEntry {
    std::string name;
    std::string hero;
    int score = 0;
    int depth = 0;
    int kills = 0;
    int gold = 0;
    int duration = 0;
};

struct OnlineLeaderboardResult {
    bool ok = false;
    std::string message;
    std::vector<OnlineLeaderboardEntry> entries;
};

class OnlineLeaderboard {
public:
    // Must run on the main thread before starting an SDL worker on PS Vita.
    static void initialize();
    static OnlineLeaderboardResult fetch();
    static OnlineLeaderboardResult submit(const RunStatistics& run);
    static const std::string& playerName();
};

} // namespace pdv
