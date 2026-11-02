#include "platform/VitaInput.h"

#include <algorithm>
#include <cmath>
#include <cstring>

namespace pdv {

namespace {
constexpr float AnalogDeadZone = 24.0f / 127.0f;
uint32_t nativeButton(GameButton button) {
    switch (button) {
        case GameButton::Up: return SCE_CTRL_UP; case GameButton::Down: return SCE_CTRL_DOWN;
        case GameButton::Left: return SCE_CTRL_LEFT; case GameButton::Right: return SCE_CTRL_RIGHT;
        case GameButton::Confirm: return SCE_CTRL_CROSS; case GameButton::Cancel: return SCE_CTRL_CIRCLE;
        case GameButton::Inventory: return SCE_CTRL_SQUARE; case GameButton::Character: return SCE_CTRL_TRIANGLE;
        case GameButton::Map: return SCE_CTRL_TRIANGLE; case GameButton::Pause: return SCE_CTRL_START;
        case GameButton::L1: return SCE_CTRL_LTRIGGER; case GameButton::R1: return SCE_CTRL_RTRIGGER;
        case GameButton::Start: return SCE_CTRL_START; case GameButton::Select: return SCE_CTRL_SELECT;
        default: return 0;
    }
}

bool containsId(const std::array<uint8_t, IInputBackend::MaxTouches>& ids,
                std::size_t count,
                uint8_t id) {
    for (std::size_t i = 0; i < count; ++i) {
        if (ids[i] == id) {
            return true;
        }
    }
    return false;
}
}

bool VitaInput::initialize() {
    std::memset(&pad_, 0, sizeof(pad_));
    std::memset(&frontPanel_, 0, sizeof(frontPanel_));

    sceCtrlSetSamplingMode(SCE_CTRL_MODE_ANALOG_WIDE);

    const int touchState = sceTouchSetSamplingState(
        SCE_TOUCH_PORT_FRONT,
        SCE_TOUCH_SAMPLING_STATE_START
    );

    if (touchState < 0) {
        return false;
    }

    const int panelResult = sceTouchGetPanelInfo(
        SCE_TOUCH_PORT_FRONT,
        &frontPanel_
    );

    return panelResult >= 0;
}

void VitaInput::shutdown() {
    sceTouchSetSamplingState(
        SCE_TOUCH_PORT_FRONT,
        SCE_TOUCH_SAMPLING_STATE_STOP
    );
}

void VitaInput::update() {
    updateController();
    updateTouch();
}

void VitaInput::updateController() {
    previousButtons_ = pad_.buttons;

    SceCtrlData next{};
    const int count = sceCtrlPeekBufferPositive(0, &next, 1);

    if (count > 0) {
        pad_ = next;
        leftX_ = normalizeAxis(pad_.lx);
        leftY_ = normalizeAxis(pad_.ly);
    }
}

void VitaInput::updateTouch() {
    std::array<uint8_t, MaxTouches> currentIds{};
    std::size_t currentCount = 0;

    for (auto& t : touches_) {
        t = TouchPoint{};
    }

    SceTouchData data{};
    const int count = sceTouchPeek(SCE_TOUCH_PORT_FRONT, &data, 1);

    if (count > 0) {
        const std::size_t reportCount =
            std::min<std::size_t>(data.reportNum, MaxTouches);

        for (std::size_t i = 0; i < reportCount; ++i) {
            const auto& report = data.report[i];
            auto& out = touches_[i];

            out.active = true;
            out.id = report.id;
            out.x = mapTouchX(report.x);
            out.y = mapTouchY(report.y);
            out.pressed = !containsId(
                previousTouchIds_,
                previousTouchCount_,
                report.id
            );

            currentIds[currentCount++] = report.id;
        }
    }

    // Keep a one-frame released marker if an old contact disappeared.
    // It is stored in the first free slot.
    for (std::size_t oldIndex = 0;
         oldIndex < previousTouchCount_;
         ++oldIndex) {

        const uint8_t oldId = previousTouchIds_[oldIndex];

        if (!containsId(currentIds, currentCount, oldId)) {
            for (auto& t : touches_) {
                if (!t.active) {
                    t.released = true;
                    t.id = oldId;
                    break;
                }
            }
        }
    }

    previousTouchIds_ = currentIds;
    previousTouchCount_ = currentCount;
}

bool VitaInput::held(GameButton button) const {
    return (pad_.buttons & nativeButton(button)) != 0;
}

bool VitaInput::pressed(GameButton button) const {
    const uint32_t native = nativeButton(button);
    return (pad_.buttons & native) != 0 &&
           (previousButtons_ & native) == 0;
}

bool VitaInput::released(GameButton button) const {
    const uint32_t native = nativeButton(button);
    return (pad_.buttons & native) == 0 &&
           (previousButtons_ & native) != 0;
}

float VitaInput::normalizeAxis(uint8_t value) const {
    float v = (static_cast<float>(value) - 128.0f) / 127.0f;

    if (std::fabs(v) < AnalogDeadZone) {
        return 0.0f;
    }

    if (v < -1.0f) v = -1.0f;
    if (v > 1.0f) v = 1.0f;
    return v;
}

float VitaInput::mapTouchX(int rawX) const {
    const float min = static_cast<float>(frontPanel_.minDispX);
    const float max = static_cast<float>(frontPanel_.maxDispX);
    const float denom = max - min;

    if (denom <= 0.0f) {
        return 0.0f;
    }

    float normalized = (static_cast<float>(rawX) - min) / denom;
    normalized = std::max(0.0f, std::min(1.0f, normalized));
    return normalized * static_cast<float>(ScreenWidth);
}

float VitaInput::mapTouchY(int rawY) const {
    const float min = static_cast<float>(frontPanel_.minDispY);
    const float max = static_cast<float>(frontPanel_.maxDispY);
    const float denom = max - min;

    if (denom <= 0.0f) {
        return 0.0f;
    }

    float normalized = (static_cast<float>(rawY) - min) / denom;
    normalized = std::max(0.0f, std::min(1.0f, normalized));
    return normalized * static_cast<float>(ScreenHeight);
}

const TouchPoint* VitaInput::primaryTouch() const {
    for (const auto& t : touches_) {
        if (t.active) {
            return &t;
        }
    }
    return nullptr;
}

} // namespace pdv
