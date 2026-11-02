#include "core/OnlineLeaderboardRequest.h"

#if !defined(PD_PLATFORM_PS3)
#include <SDL2/SDL.h>
#endif

#include <memory>
#include <utility>

namespace pdv {

struct OnlineLeaderboardRequest::State {
#if !defined(PD_PLATFORM_PS3)
    SDL_mutex* mutex = nullptr;
#endif
    bool submit = false;
    RunStatistics run{};
    bool ready = false;
    OnlineLeaderboardResult result;

    ~State() {
#if !defined(PD_PLATFORM_PS3)
        if (mutex != nullptr) {
            SDL_DestroyMutex(mutex);
        }
#endif
    }
};

bool OnlineLeaderboardRequest::startFetch() {
    if (!OnlineServiceEnabled) return false;
    OnlineLeaderboard::initialize();
    return start(false, nullptr);
}

bool OnlineLeaderboardRequest::startSubmit(const RunStatistics& run) {
    if (!OnlineServiceEnabled) return false;
    OnlineLeaderboard::initialize();
    return start(true, &run);
}

bool OnlineLeaderboardRequest::start(bool submit, const RunStatistics* run) {
    if (!OnlineServiceEnabled) return false;
    if (state_) {
        return false;
    }

    auto state = std::make_shared<State>();
#if defined(PD_PLATFORM_PS3)
    // The PS3 build deliberately keeps the unavailable online service on the
    // main thread.  This removes SDL's thread/mutex layer from the PS3 target;
    // OnlineLeaderboard itself returns a prompt "unavailable" result there.
    state->submit = submit;
    if (run != nullptr) state->run = *run;
    state->result = submit ? OnlineLeaderboard::submit(state->run) : OnlineLeaderboard::fetch();
    state->ready = true;
    state_ = std::move(state);
    return true;
#else
    state->mutex = SDL_CreateMutex();
    if (state->mutex == nullptr) {
        state->ready = true;
        state->result.message = "Network unavailable";
        state_ = std::move(state);
        return false;
    }

    state->submit = submit;
    if (run != nullptr) {
        state->run = *run;
    }

    // The worker owns an independent shared_ptr, so a scene can close while a
    // request is still in flight without leaving it with a dangling pointer.
    auto* workerState = new std::shared_ptr<State>(state);
    SDL_Thread* thread = SDL_CreateThread(&OnlineLeaderboardRequest::run, "OnlineLeaderboard", workerState);
    if (thread == nullptr) {
        delete workerState;
        state->ready = true;
        state->result.message = "Network unavailable";
        state_ = std::move(state);
        return false;
    }

    SDL_DetachThread(thread);
    state_ = std::move(state);
    return true;
#endif
}

int OnlineLeaderboardRequest::run(void* data) {
#if defined(PD_PLATFORM_PS3)
    (void)data;
    return 0;
#else
    std::unique_ptr<std::shared_ptr<State>> owner(static_cast<std::shared_ptr<State>*>(data));
    const std::shared_ptr<State> state = *owner;

    OnlineLeaderboardResult result;
    try {
        result = state->submit ? OnlineLeaderboard::submit(state->run) : OnlineLeaderboard::fetch();
    } catch (...) {
        result.message = "Online service unavailable";
    }

    SDL_LockMutex(state->mutex);
    state->result = std::move(result);
    state->ready = true;
    SDL_UnlockMutex(state->mutex);
    return 0;
#endif
}

bool OnlineLeaderboardRequest::poll(OnlineLeaderboardResult& result) {
    if (!state_) {
        return false;
    }

#if !defined(PD_PLATFORM_PS3)
    SDL_LockMutex(state_->mutex);
#endif
    const bool ready = state_->ready;
    if (ready) {
        result = std::move(state_->result);
    }
#if !defined(PD_PLATFORM_PS3)
    SDL_UnlockMutex(state_->mutex);
#endif

    if (ready) {
        state_.reset();
    }
    return ready;
}

void OnlineLeaderboardRequest::cancel() {
    state_.reset();
}

} // namespace pdv
