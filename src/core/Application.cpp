#include "core/Application.h"
#include "assets/AssetCatalog.h"
#include "audio/AudioSystem.h"
#include "game/Progression.h"

#include <chrono>
#if defined(PD_PLATFORM_PSP)
#include <pspkernel.h>
#endif
#if defined(PD_PLATFORM_PS2)
#include <cstdio>
#endif
#if defined(PD_PLATFORM_WINDOWS)
#include <fstream>
#include <iomanip>
#endif

namespace pdv {

bool Application::initialize() {
    initializationError_ = "unknown";
    Progression::instance().load();
    if (!input_.initialize()) {
        initializationError_ = "controller";
#if defined(PD_PLATFORM_PS2)
        std::printf("[PDPS2] initialization failed: controller\n");
#endif
        return false;
    }

    if (!renderer_.initialize()) {
        initializationError_ = "SDL renderer";
#if defined(PD_PLATFORM_PS2)
        std::printf("[PDPS2] initialization failed: SDL renderer\n");
#endif
        return false;
    }

    AudioSystem::instance().initialize();

    if (!font_.initialize(textures_.get(Assets::UIFontAtlas))) {
        initializationError_ = textures_.lastError().empty() ? "UI font/resources" : textures_.lastError().c_str();
#if defined(PD_PLATFORM_PS2)
        std::printf("[PDPS2] initialization failed: UI font/resources\n");
#endif
        return false;
    }

    if (!titleScene_.initialize(textures_, font_)) {
        initializationError_ = "title scene/resources";
#if defined(PD_PLATFORM_PS2)
        std::printf("[PDPS2] initialization failed: title scene/resources\n");
#endif
        return false;
    }

    if (!startScene_.initialize(textures_, font_)) {
        initializationError_ = "start scene/resources";
#if defined(PD_PLATFORM_PS2)
        std::printf("[PDPS2] initialization failed: start scene/resources\n");
#endif
        return false;
    }

    if (!gameScene_.initialize(textures_, font_)) {
        initializationError_ = "game scene/resources";
#if defined(PD_PLATFORM_PS2)
        std::printf("[PDPS2] initialization failed: game scene/resources\n");
#endif
        return false;
    }

    titleScene_.onEnter();
    initializationError_ = "";
    running_ = true;
    return true;
}

void Application::run() {
#if defined(PD_PLATFORM_PSP)
    // The PSP libstdc++ steady_clock may remain at a fixed value. That leaves
    // time-limited overlays active forever, consuming all gameplay input.
    uint64_t previousTime = sceKernelGetSystemTimeWide();
#else
    auto previousTime = std::chrono::steady_clock::now();
#endif
#if defined(PD_PLATFORM_WINDOWS)
    std::ofstream frameProfile("frame_profile.csv", std::ios::trunc);
    if (frameProfile) frameProfile << "frame_ms,input_ms,update_ms,transition_ms,render_ms,scene\n";
#endif

#if defined(PD_PLATFORM_PS2)
    // A PS2 ELF has no desktop window-close event. Keep the game loop owned by
    // the executable so transient pad/SDL state cannot hand control to OSDSYS.
    while (true) {
#else
    while (running_) {
#endif
#if defined(PD_PLATFORM_PSP)
        const uint64_t now = sceKernelGetSystemTimeWide();
        float dt = static_cast<float>(now - previousTime) / 1000000.0f;
        previousTime = now;
#else
        const auto now = std::chrono::steady_clock::now();
        float dt = std::chrono::duration<float>(now - previousTime).count();
        previousTime = now;
#endif

        if (dt < 0.0f) {
            dt = 0.0f;
        } else if (dt > 0.10f) {
            dt = 0.10f;
        }

        const auto frameStart = now;
        input_.update();
        const auto afterInput = std::chrono::steady_clock::now();

        bool shouldQuit = input_.quitRequested();
#if !defined(PD_PLATFORM_PS2)
        shouldQuit = shouldQuit ||
            (input_.held(GameButton::Start) && input_.pressed(GameButton::Select));
#endif
        if (shouldQuit) {
            running_ = false;
        }

        update(dt);
        AudioSystem::instance().update();
        const auto afterUpdate = std::chrono::steady_clock::now();
        processSceneTransition();
        const auto afterTransition = std::chrono::steady_clock::now();
        render();
        const auto afterRender = std::chrono::steady_clock::now();

#if defined(PD_PLATFORM_WINDOWS)
        const auto ms = [](const auto& from, const auto& to) {
            return std::chrono::duration<double, std::milli>(to - from).count();
        };
        const double frameMs = ms(frameStart, afterRender);
        // Persist only noticeable hitches, so the profiler itself cannot create
        // the stutters it is meant to diagnose.
        if (frameProfile && frameMs >= 20.0) {
            frameProfile << std::fixed << std::setprecision(3)
                         << frameMs << ',' << ms(frameStart, afterInput) << ','
                         << ms(afterInput, afterUpdate) << ','
                         << ms(afterUpdate, afterTransition) << ','
                         << ms(afterTransition, afterRender) << ','
                         << static_cast<int>(scenes_.current()) << std::endl;
        }
#endif
    }
}

void Application::update(float dt) {
    switch (scenes_.current()) {
        case SceneId::Title:
            titleScene_.update(dt, input_, scenes_);
            break;

        case SceneId::Start:
            startScene_.update(dt, input_, scenes_, session_);
            break;

        case SceneId::Game:
            gameScene_.update(dt, input_, scenes_, session_);
            break;
    }
}

void Application::render() {
    renderer_.beginFrame();

    switch (scenes_.current()) {
        case SceneId::Title:
            titleScene_.render(renderer_);
            break;

        case SceneId::Start:
            startScene_.render(renderer_);
            break;

        case SceneId::Game:
            gameScene_.render(renderer_);
            break;
    }

    renderer_.present();
}

void Application::processSceneTransition() {
    if (!scenes_.hasPending()) {
        return;
    }

    const SceneId oldScene = scenes_.current();

    switch (oldScene) {
        case SceneId::Title:
            titleScene_.onExit();
            break;

        case SceneId::Start:
            startScene_.onExit();
            break;

        case SceneId::Game:
            gameScene_.onExit();
            break;
    }

    const SceneId nextScene = scenes_.consumePending();

    switch (nextScene) {
        case SceneId::Title:
            titleScene_.onEnter();
            break;

        case SceneId::Start:
            startScene_.onEnter(session_);
            break;

        case SceneId::Game:
            gameScene_.onEnter(session_);
            break;
    }
}

void Application::shutdown() {
    Progression::instance().save();
    titleScene_.shutdown();
    startScene_.shutdown();
    gameScene_.shutdown();

    textures_.clear();
    AudioSystem::instance().shutdown();
    input_.shutdown();
    renderer_.shutdown();

}

} // namespace pdv
