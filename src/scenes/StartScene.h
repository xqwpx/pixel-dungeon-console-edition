#pragma once

#include <array>

#include "core/GameSession.h"
#include "render/Sprite.h"
#include "ui/HeroShield.h"
#include "ui/TextButton.h"

namespace pdv {

class BitmapFont;
class Renderer2D;
class SceneManager;
class Texture2D;
class TextureCache;
class IInputBackend;

class StartScene {
public:
    bool initialize(TextureCache& textures, BitmapFont& font);
    void shutdown();

    void onEnter(const GameSession& session);
    void onExit();

    void update(
        float dt,
        const IInputBackend& input,
        SceneManager& scenes,
        GameSession& session
    );

    void render(Renderer2D& renderer);

private:
    void moveSelection(int delta);

    void launchSelected(
        SceneManager& scenes,
        GameSession& session
    );

    void launchSelectedAction(
        SceneManager& scenes,
        GameSession& session
    );

    void handleButtons(
        const IInputBackend& input,
        SceneManager& scenes,
        GameSession& session
    );

    void handleAnalog(const IInputBackend& input);

    void handleTouch(
        const IInputBackend& input,
        SceneManager& scenes,
        GameSession& session
    );

    int heroIndexAt(float x, float y) const;

    Texture2D* banners_ = nullptr;
    Texture2D* avatars_ = nullptr;
    std::array<Texture2D*, 4> heroBackgrounds_{};
    Texture2D* chrome_ = nullptr;
    Texture2D* uiButtons_ = nullptr;

    BitmapFont* font_ = nullptr;

    Sprite selectHeroBanner_;

    std::array<HeroShield, 4> heroes_;

    TextButton newGameButton_;
    TextButton continueButton_;

    int selectedHero_ = 0;
    int selectedAction_ = 0;
    int touchedHero_ = -1;

    bool analogLatched_ = false;
    bool saveAvailable_ = false;

};

} // namespace pdv
