#include "platform/windows/WindowsInput.h"
#include <SDL2/SDL.h>

#include <cmath>

namespace pdv {
namespace {

constexpr std::size_t index(GameButton b) { return static_cast<std::size_t>(b); }
constexpr Sint16 StickDeadZone = 8192;

float normalizedAxis(Sint16 value) {
    const float normalized = static_cast<float>(value) / 32767.0f;
    return normalized < -1.0f ? -1.0f : (normalized > 1.0f ? 1.0f : normalized);
}

} // namespace

bool WindowsInput::initialize() {
    controllerSubsystemReady_ = SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER) == 0;
    if (controllerSubsystemReady_) {
        openFirstController();
    }
    return true;
}

void WindowsInput::shutdown() {
    closeController();
    if (controllerSubsystemReady_) {
        SDL_QuitSubSystem(SDL_INIT_GAMECONTROLLER);
        controllerSubsystemReady_ = false;
    }
}

void WindowsInput::openFirstController() {
    if (controller_ != nullptr) {
        return;
    }

    for (int deviceIndex = 0; deviceIndex < SDL_NumJoysticks(); ++deviceIndex) {
        if (!SDL_IsGameController(deviceIndex)) {
            continue;
        }

        controller_ = SDL_GameControllerOpen(deviceIndex);
        if (controller_ != nullptr) {
            controllerInstanceId_ = SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(controller_));
            return;
        }
    }
}

void WindowsInput::closeController() {
    if (controller_ != nullptr) {
        SDL_GameControllerClose(controller_);
        controller_ = nullptr;
    }
    controllerInstanceId_ = -1;
}

void WindowsInput::update() {
    previous_ = current_; current_.fill(false); leftX_ = leftY_ = 0.0f;
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            quitRequested_ = true;
        } else if (event.type == SDL_CONTROLLERDEVICEADDED && controller_ == nullptr) {
            openFirstController();
        } else if (event.type == SDL_CONTROLLERDEVICEREMOVED &&
                   event.cdevice.which == controllerInstanceId_) {
            closeController();
            openFirstController();
        }
    }

    const Uint8* k = SDL_GetKeyboardState(nullptr);
    // Desktop layout mirrors the four face-button actions shown by the game.
    // Use scancodes so right-side modifiers work regardless of keyboard layout.
    current_[index(GameButton::Up)] = k[SDL_SCANCODE_UP];
    current_[index(GameButton::Down)] = k[SDL_SCANCODE_DOWN];
    current_[index(GameButton::Left)] = k[SDL_SCANCODE_LEFT];
    current_[index(GameButton::Right)] = k[SDL_SCANCODE_RIGHT];
    current_[index(GameButton::Confirm)] = k[SDL_SCANCODE_SPACE];
    current_[index(GameButton::Cancel)] = k[SDL_SCANCODE_RALT];
    current_[index(GameButton::Inventory)] = k[SDL_SCANCODE_RCTRL];
    current_[index(GameButton::Character)] = k[SDL_SCANCODE_RSHIFT];
    current_[index(GameButton::Map)] = k[SDL_SCANCODE_M];
    current_[index(GameButton::Pause)] = k[SDL_SCANCODE_ESCAPE];
    current_[index(GameButton::L1)] = k[SDL_SCANCODE_Q];
    current_[index(GameButton::R1)] = k[SDL_SCANCODE_RETURN];
    current_[index(GameButton::Start)] = k[SDL_SCANCODE_ESCAPE];
    current_[index(GameButton::Select)] = k[SDL_SCANCODE_TAB];

    if (controller_ == nullptr && controllerSubsystemReady_) {
        openFirstController();
    }

    if (controller_ == nullptr) {
        return;
    }

    const auto button = [this](SDL_GameControllerButton value) {
        return SDL_GameControllerGetButton(controller_, value) != 0;
    };
    const Sint16 axisX = SDL_GameControllerGetAxis(controller_, SDL_CONTROLLER_AXIS_LEFTX);
    const Sint16 axisY = SDL_GameControllerGetAxis(controller_, SDL_CONTROLLER_AXIS_LEFTY);
    leftX_ = std::abs(axisX) >= StickDeadZone ? normalizedAxis(axisX) : 0.0f;
    leftY_ = std::abs(axisY) >= StickDeadZone ? normalizedAxis(axisY) : 0.0f;

    current_[index(GameButton::Up)] |= button(SDL_CONTROLLER_BUTTON_DPAD_UP) || leftY_ <= -0.65f;
    current_[index(GameButton::Down)] |= button(SDL_CONTROLLER_BUTTON_DPAD_DOWN) || leftY_ >= 0.65f;
    current_[index(GameButton::Left)] |= button(SDL_CONTROLLER_BUTTON_DPAD_LEFT) || leftX_ <= -0.65f;
    current_[index(GameButton::Right)] |= button(SDL_CONTROLLER_BUTTON_DPAD_RIGHT) || leftX_ >= 0.65f;
    current_[index(GameButton::Confirm)] |= button(SDL_CONTROLLER_BUTTON_A);
    current_[index(GameButton::Cancel)] |= button(SDL_CONTROLLER_BUTTON_B);
    current_[index(GameButton::Inventory)] |= button(SDL_CONTROLLER_BUTTON_X);
    current_[index(GameButton::Character)] |= button(SDL_CONTROLLER_BUTTON_Y);
    current_[index(GameButton::Map)] |= button(SDL_CONTROLLER_BUTTON_Y);
    current_[index(GameButton::Pause)] |= button(SDL_CONTROLLER_BUTTON_START);
    current_[index(GameButton::L1)] |= button(SDL_CONTROLLER_BUTTON_LEFTSHOULDER);
    current_[index(GameButton::R1)] |= button(SDL_CONTROLLER_BUTTON_RIGHTSHOULDER);
    current_[index(GameButton::L2)] |= SDL_GameControllerGetAxis(controller_, SDL_CONTROLLER_AXIS_TRIGGERLEFT) > StickDeadZone;
    current_[index(GameButton::R2)] |= SDL_GameControllerGetAxis(controller_, SDL_CONTROLLER_AXIS_TRIGGERRIGHT) > StickDeadZone;
    current_[index(GameButton::Start)] |= button(SDL_CONTROLLER_BUTTON_START);
    current_[index(GameButton::Select)] |= button(SDL_CONTROLLER_BUTTON_BACK);
}
bool WindowsInput::held(GameButton b) const { return current_[index(b)]; }
bool WindowsInput::pressed(GameButton b) const { return current_[index(b)] && !previous_[index(b)]; }
bool WindowsInput::released(GameButton b) const { return !current_[index(b)] && previous_[index(b)]; }
const TouchPoint* WindowsInput::primaryTouch() const { return nullptr; }
} // namespace pdv
