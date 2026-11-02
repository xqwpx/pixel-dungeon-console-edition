#include "platform/ps1/PS1Input.h"

extern "C" {
#include <psxapi.h>
#include <psxpad.h>
}

namespace pdv {

namespace {

constexpr unsigned int index(PS1Button button) {
    return static_cast<unsigned int>(button);
}

} // namespace

bool PS1Input::initialize() {
    InitPAD(padBuffers_[0], sizeof(padBuffers_[0]), padBuffers_[1], sizeof(padBuffers_[1]));
    StartPAD();
    ChangeClearPAD(0);
    for (unsigned int index = 0; index < ButtonCount; ++index) {
        current_[index] = false;
        previous_[index] = false;
    }
    initialized_ = true;
    return true;
}

void PS1Input::shutdown() {
    if (initialized_) {
        StopPAD();
        initialized_ = false;
    }
}

void PS1Input::update() {
    for (unsigned int index = 0; index < ButtonCount; ++index) {
        previous_[index] = current_[index];
        current_[index] = false;
    }
    leftX_ = 0.0f;
    leftY_ = 0.0f;
    if (!initialized_) return;

    const auto* pad = reinterpret_cast<const PADTYPE*>(padBuffers_[0]);
    // BIOS pad data stores pressed buttons as zero bits. A non-zero status
    // means no stable controller response is available this frame.
    if (pad->stat != 0) return;
    const unsigned int buttons = static_cast<unsigned int>(~pad->btn) & 0xffffu;
    const auto down = [buttons](unsigned int bit) { return (buttons & bit) != 0; };

    current_[index(PS1Button::Up)] = down(PAD_UP);
    current_[index(PS1Button::Down)] = down(PAD_DOWN);
    current_[index(PS1Button::Left)] = down(PAD_LEFT);
    current_[index(PS1Button::Right)] = down(PAD_RIGHT);
    current_[index(PS1Button::Confirm)] = down(PAD_CROSS);
    current_[index(PS1Button::Cancel)] = down(PAD_CIRCLE);
    current_[index(PS1Button::Inventory)] = down(PAD_SQUARE);
    current_[index(PS1Button::Character)] = down(PAD_TRIANGLE);
    current_[index(PS1Button::Map)] = down(PAD_SELECT);
    current_[index(PS1Button::Pause)] = down(PAD_START);
    current_[index(PS1Button::L1)] = down(PAD_L1);
    current_[index(PS1Button::R1)] = down(PAD_R1);
    current_[index(PS1Button::L2)] = down(PAD_L2);
    current_[index(PS1Button::R2)] = down(PAD_R2);
    current_[index(PS1Button::Start)] = down(PAD_START);
    current_[index(PS1Button::Select)] = down(PAD_SELECT);

    if (pad->type == PAD_ID_ANALOG || pad->type == PAD_ID_ANALOG_STICK) {
        leftX_ = (static_cast<int>(pad->ls_x) - 128) / 127.0f;
        leftY_ = (static_cast<int>(pad->ls_y) - 128) / 127.0f;
    }
}

bool PS1Input::held(PS1Button button) const { return current_[index(button)]; }
bool PS1Input::pressed(PS1Button button) const { return current_[index(button)] && !previous_[index(button)]; }
bool PS1Input::released(PS1Button button) const { return !current_[index(button)] && previous_[index(button)]; }

} // namespace pdv
