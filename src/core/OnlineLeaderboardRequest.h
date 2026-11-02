#pragma once

#include <memory>

#include "core/OnlineLeaderboard.h"

namespace pdv {

// Keep the implementation in the project, but make every online request a
// no-op until the service is ready to be enabled again.
inline constexpr bool OnlineServiceEnabled = false;

// SDL threads are supported by the Vita SDL port.  Do not use std::async here:
// libstdc++ fails to create its worker thread on hardware and throws instead.
class OnlineLeaderboardRequest {
public:
    bool startFetch();
    bool startSubmit(const RunStatistics& run);

    // Returns true exactly once, when a completed result has been written.
    bool poll(OnlineLeaderboardResult& result);
    void cancel();

private:
    struct State;

    bool start(bool submit, const RunStatistics* run);
    static int run(void* data);

    std::shared_ptr<State> state_;
};

} // namespace pdv
