#include "platform/ps2/PS2Input.h"

#include <cmath>
#include <libpad.h>

namespace pdv {
namespace {
constexpr std::size_t index(GameButton button) { return static_cast<std::size_t>(button); }
constexpr float DeadZone = 0.25f;
float axis(unsigned char value) {
    const float result = (static_cast<float>(value) - 128.0f) / 127.0f;
    return std::max(-1.0f, std::min(1.0f, result));
}
}

bool PS2Input::initialize() {
    if (padInit(0) != 1) return false;
    padOpen_ = padPortOpen(0, 0, padBuffer_.data()) != 0;
    return padOpen_;
}

void PS2Input::shutdown() {
    if (padOpen_) padPortClose(0, 0);
    padOpen_ = false;
    padEnd();
}

void PS2Input::update() {
    previous_ = current_;
    if (!padOpen_) { current_.fill(false); return; }
    const int state = padGetState(0, 0);
    if (state != PAD_STATE_STABLE && state != PAD_STATE_FINDCTP1) {
        current_.fill(false);
        leftX_ = 0.0f;
        leftY_ = 0.0f;
        return;
    }

    padButtonStatus data{};
    if (!padRead(0, 0, &data)) {
        current_.fill(false);
        leftX_ = 0.0f;
        leftY_ = 0.0f;
        return;
    }
    current_.fill(false);
    const unsigned short buttons = static_cast<unsigned short>(~data.btns);
    // Digital pads do not report analogue stick coordinates. libpad returns
    // (0, 0) in that mode, which is not a real stick direction: interpreting
    // it as signed axes permanently holds Left and Up.
    if (data.ljoy_h == 0 && data.ljoy_v == 0) {
        leftX_ = 0.0f;
        leftY_ = 0.0f;
    } else {
        leftX_ = axis(data.ljoy_h);
        leftY_ = axis(data.ljoy_v);
    }
    if (std::abs(leftX_) < DeadZone) leftX_ = 0.0f;
    if (std::abs(leftY_) < DeadZone) leftY_ = 0.0f;
    current_[index(GameButton::Up)] = (buttons & PAD_UP) || leftY_ <= -0.65f;
    current_[index(GameButton::Down)] = (buttons & PAD_DOWN) || leftY_ >= 0.65f;
    current_[index(GameButton::Left)] = (buttons & PAD_LEFT) || leftX_ <= -0.65f;
    current_[index(GameButton::Right)] = (buttons & PAD_RIGHT) || leftX_ >= 0.65f;
    current_[index(GameButton::Confirm)] = buttons & PAD_CROSS;
    current_[index(GameButton::Cancel)] = buttons & PAD_CIRCLE;
    current_[index(GameButton::Inventory)] = buttons & PAD_SQUARE;
    current_[index(GameButton::Character)] = buttons & PAD_TRIANGLE;
    current_[index(GameButton::Map)] = buttons & PAD_TRIANGLE;
    current_[index(GameButton::Pause)] = buttons & PAD_START;
    current_[index(GameButton::L1)] = buttons & PAD_L1;
    current_[index(GameButton::R1)] = buttons & PAD_R1;
    current_[index(GameButton::L2)] = buttons & PAD_L2;
    current_[index(GameButton::R2)] = buttons & PAD_R2;
    current_[index(GameButton::Start)] = buttons & PAD_START;
    current_[index(GameButton::Select)] = buttons & PAD_SELECT;
}

bool PS2Input::held(GameButton button) const { return current_[index(button)]; }
bool PS2Input::pressed(GameButton button) const { return current_[index(button)] && !previous_[index(button)]; }
bool PS2Input::released(GameButton button) const { return !current_[index(button)] && previous_[index(button)]; }
} // namespace pdv
