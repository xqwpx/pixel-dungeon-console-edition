#include "platform/ps3/PS3Input.h"

#include <cmath>
#include <io/pad.h>

namespace pdv {
namespace {
constexpr std::size_t index(GameButton button) { return static_cast<std::size_t>(button); }
constexpr float StickDeadZone = 0.25f;

float normalizeAxis(unsigned int value) {
    const float normalized = (static_cast<float>(value) - 128.0f) / 127.0f;
    return normalized < -1.0f ? -1.0f : (normalized > 1.0f ? 1.0f : normalized);
}
}

bool PS3Input::initialize() {
    const s32 result = ioPadInit(MAX_PORT_NUM);
    padReady_ = result == PAD_OK || result == static_cast<s32>(PAD_ERROR_ALREADY_INITIALIZED);
    return padReady_;
}

void PS3Input::shutdown() {
    if (padReady_) ioPadEnd();
    padReady_ = false;
    activePort_ = -1;
}

void PS3Input::update() {
    previous_ = current_;

    if (!padReady_) {
        current_.fill(false);
        leftX_ = leftY_ = 0.0f;
        return;
    }

    padInfo info{};
    if (ioPadGetInfo(&info) != PAD_OK) return;
    activePort_ = -1;
    for (int port = 0; port < MAX_PORT_NUM; ++port) {
        if (info.status[port]) {
            activePort_ = port;
            break;
        }
    }
    if (activePort_ < 0) {
        current_.fill(false);
        leftX_ = leftY_ = 0.0f;
        return;
    }

    padData data{};
    // len == 0 means "no new controller packet", not "all buttons were
    // released". Clearing current_ here creates a false release/press pair
    // and made a physical press appear twice when frame time increased.
    if (ioPadGetData(static_cast<u32>(activePort_), &data) != PAD_OK || data.len == 0) return;

    current_.fill(false);

    leftX_ = normalizeAxis(data.ANA_L_H);
    leftY_ = normalizeAxis(data.ANA_L_V);
    if (std::abs(leftX_) < StickDeadZone) leftX_ = 0.0f;
    if (std::abs(leftY_) < StickDeadZone) leftY_ = 0.0f;

    current_[index(GameButton::Up)] = data.BTN_UP || leftY_ <= -0.65f;
    current_[index(GameButton::Down)] = data.BTN_DOWN || leftY_ >= 0.65f;
    current_[index(GameButton::Left)] = data.BTN_LEFT || leftX_ <= -0.65f;
    current_[index(GameButton::Right)] = data.BTN_RIGHT || leftX_ >= 0.65f;
    current_[index(GameButton::Confirm)] = data.BTN_CROSS;
    current_[index(GameButton::Cancel)] = data.BTN_CIRCLE;
    current_[index(GameButton::Inventory)] = data.BTN_SQUARE;
    current_[index(GameButton::Character)] = data.BTN_TRIANGLE;
    current_[index(GameButton::Map)] = data.BTN_TRIANGLE;
    current_[index(GameButton::Pause)] = data.BTN_START;
    current_[index(GameButton::L1)] = data.BTN_L1;
    current_[index(GameButton::R1)] = data.BTN_R1;
    current_[index(GameButton::L2)] = data.BTN_L2;
    current_[index(GameButton::R2)] = data.BTN_R2;
    current_[index(GameButton::Start)] = data.BTN_START;
    current_[index(GameButton::Select)] = data.BTN_SELECT;
}

bool PS3Input::held(GameButton button) const { return current_[index(button)]; }
bool PS3Input::pressed(GameButton button) const { return current_[index(button)] && !previous_[index(button)]; }
bool PS3Input::released(GameButton button) const { return !current_[index(button)] && previous_[index(button)]; }

} // namespace pdv
