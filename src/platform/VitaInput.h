#pragma once

#include <array>
#include <cstdint>
#include "engine/input/IInputBackend.h"
#include <psp2/ctrl.h>
#include <psp2/touch.h>

namespace pdv {

class VitaInput final : public IInputBackend {
public:
    static constexpr int ScreenWidth = 960;
    static constexpr int ScreenHeight = 544;
    static constexpr std::size_t VitaMaxTouches = SCE_TOUCH_MAX_REPORT;

    bool initialize() override;
    void shutdown() override;
    void update() override;

    bool held(GameButton button) const override;
    bool pressed(GameButton button) const override;
    bool released(GameButton button) const override;

    float leftX() const override { return leftX_; }
    float leftY() const override { return leftY_; }

    const std::array<TouchPoint, MaxTouches>& touches() const override {
        return touches_;
    }

    const TouchPoint* primaryTouch() const override;
    bool quitRequested() const override { return false; }

private:
    void updateController();
    void updateTouch();

    float normalizeAxis(uint8_t value) const;
    float mapTouchX(int rawX) const;
    float mapTouchY(int rawY) const;

    SceCtrlData pad_{};
    uint32_t previousButtons_ = 0;

    float leftX_ = 0.0f;
    float leftY_ = 0.0f;

    SceTouchPanelInfo frontPanel_{};
    std::array<TouchPoint, MaxTouches> touches_{};
    std::array<uint8_t, MaxTouches> previousTouchIds_{};
    std::size_t previousTouchCount_ = 0;
};

} // namespace pdv
