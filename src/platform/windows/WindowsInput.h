#pragma once
#include "engine/input/IInputBackend.h"

#include <SDL2/SDL.h>

namespace pdv {
class WindowsInput final : public IInputBackend {
public:
    bool initialize() override;
    void shutdown() override;
    void update() override;
    bool held(GameButton) const override;
    bool pressed(GameButton) const override;
    bool released(GameButton) const override;
    float leftX() const override { return leftX_; }
    float leftY() const override { return leftY_; }
    const std::array<TouchPoint, MaxTouches>& touches() const override { return touches_; }
    const TouchPoint* primaryTouch() const override;
    bool quitRequested() const override { return quitRequested_; }
private:
    void openFirstController();
    void closeController();

    std::array<bool, static_cast<std::size_t>(GameButton::Count)> current_{};
    std::array<bool, static_cast<std::size_t>(GameButton::Count)> previous_{};
    std::array<TouchPoint, MaxTouches> touches_{};
    SDL_GameController* controller_ = nullptr;
    SDL_JoystickID controllerInstanceId_ = -1;
    float leftX_ = 0.0f, leftY_ = 0.0f;
    bool quitRequested_ = false;
    bool controllerSubsystemReady_ = false;
};
} // namespace pdv
