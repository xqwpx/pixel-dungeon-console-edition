#pragma once

#include "assets/TextureCache.h"
#include "core/GameSession.h"
#include "core/SceneManager.h"
#include "engine/input/IInputBackend.h"
#include "render/BitmapFont.h"
#include "render/Renderer2D.h"
#include "scenes/GameScene.h"
#include "scenes/StartScene.h"
#include "scenes/TitleScene.h"

namespace pdv {

class Application {
public:
    explicit Application(IInputBackend& input) : input_(input) {}
    bool initialize();
    const char* initializationError() const { return initializationError_; }
    void run();
    void shutdown();

private:
    void update(float dt);
    void render();
    void processSceneTransition();

    bool running_ = false;
    const char* initializationError_ = "unknown";

    IInputBackend& input_;
    Renderer2D renderer_;
    TextureCache textures_;
    BitmapFont font_;

    SceneManager scenes_;
    GameSession session_;

    TitleScene titleScene_;
    StartScene startScene_;
    GameScene gameScene_;
};

} // namespace pdv
