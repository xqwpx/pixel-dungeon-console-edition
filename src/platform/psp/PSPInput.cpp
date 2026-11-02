#include "platform/psp/PSPInput.h"

#include <algorithm>
#include <cmath>
#include <pspctrl.h>

namespace pdv {
namespace {
constexpr std::size_t index(GameButton button) { return static_cast<std::size_t>(button); }
constexpr float DeadZone = 0.25f;

float axis(unsigned char value) {
    return std::max(-1.0f, std::min(1.0f, (static_cast<float>(value) - 128.0f) / 127.0f));
}
}

bool PSPInput::initialize() {
    sceCtrlSetSamplingCycle(0);
    sceCtrlSetSamplingMode(PSP_CTRL_MODE_ANALOG);
    return true;
}

void PSPInput::shutdown() {}

void PSPInput::update() {
    previous_ = current_;
    current_.fill(false);

    SceCtrlData pad{};
    if (sceCtrlPeekBufferPositive(&pad, 1) != 1) {
        leftX_ = leftY_ = 0.0f;
        return;
    }

    leftX_ = axis(pad.Lx);
    leftY_ = axis(pad.Ly);
    if (std::abs(leftX_) < DeadZone) leftX_ = 0.0f;
    if (std::abs(leftY_) < DeadZone) leftY_ = 0.0f;
    const unsigned int buttons = pad.Buttons;
    current_[index(GameButton::Up)] = (buttons & PSP_CTRL_UP) || leftY_ <= -0.65f;
    current_[index(GameButton::Down)] = (buttons & PSP_CTRL_DOWN) || leftY_ >= 0.65f;
    current_[index(GameButton::Left)] = (buttons & PSP_CTRL_LEFT) || leftX_ <= -0.65f;
    current_[index(GameButton::Right)] = (buttons & PSP_CTRL_RIGHT) || leftX_ >= 0.65f;
    current_[index(GameButton::Confirm)] = buttons & PSP_CTRL_CROSS;
    current_[index(GameButton::Cancel)] = buttons & PSP_CTRL_CIRCLE;
    current_[index(GameButton::Inventory)] = buttons & PSP_CTRL_SQUARE;
    current_[index(GameButton::Character)] = buttons & PSP_CTRL_TRIANGLE;
    current_[index(GameButton::Map)] = buttons & PSP_CTRL_TRIANGLE;
    current_[index(GameButton::Pause)] = buttons & PSP_CTRL_START;
    current_[index(GameButton::L1)] = buttons & PSP_CTRL_LTRIGGER;
    current_[index(GameButton::R1)] = buttons & PSP_CTRL_RTRIGGER;
    current_[index(GameButton::Start)] = buttons & PSP_CTRL_START;
    current_[index(GameButton::Select)] = buttons & PSP_CTRL_SELECT;
}

bool PSPInput::held(GameButton button) const { return current_[index(button)]; }
bool PSPInput::pressed(GameButton button) const { return current_[index(button)] && !previous_[index(button)]; }
bool PSPInput::released(GameButton button) const { return !current_[index(button)] && previous_[index(button)]; }
} // namespace pdv
