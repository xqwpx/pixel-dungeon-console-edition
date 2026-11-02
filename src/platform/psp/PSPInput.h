#pragma once

#include "engine/input/IInputBackend.h"

namespace pdv {

class PSPInput final : public IInputBackend {
public:
    bool initialize() override;
    void shutdown() override;
    void update() override;
    bool held(GameButton button) const override;
    bool pressed(GameButton button) const override;
    bool released(GameButton button) const override;
    float leftX() const override { return leftX_; }
    float leftY() const override { return leftY_; }
    const std::array<TouchPoint, MaxTouches>& touches() const override { return touches_; }
    const TouchPoint* primaryTouch() const override { return nullptr; }
    bool quitRequested() const override { return false; }

private:
    std::array<bool, static_cast<std::size_t>(GameButton::Count)> current_{};
    std::array<bool, static_cast<std::size_t>(GameButton::Count)> previous_{};
    std::array<TouchPoint, MaxTouches> touches_{};
    float leftX_ = 0.0f;
    float leftY_ = 0.0f;
};

} // namespace pdv
