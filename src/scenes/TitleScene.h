#pragma once

#include <array>
#include <string>

#include "core/OnlineLeaderboard.h"
#include "core/OnlineLeaderboardRequest.h"

#include "render/Sprite.h"
#include "ui/DashboardButton.h"

namespace pdv {

class BitmapFont;
class Renderer2D;
class SceneManager;
class Texture2D;
class TextureCache;
class IInputBackend;

class TitleScene {
public:
    bool initialize(TextureCache& textures, BitmapFont& font);
    void shutdown();

    void onEnter();
    void onExit();

    void update(
        float dt,
        const IInputBackend& input,
        SceneManager& scenes
    );

    void render(Renderer2D& renderer);

private:
    enum class Item : int {
        Play = 0,
        Rankings = 1,
        Badges = 2,
        About = 3,
        Settings = 4,
        OnlineLeaderboard = 5
    };

    void moveSelection(int delta);

    void activate(
        Item item,
        SceneManager& scenes
    );

    void handleButtons(
        const IInputBackend& input,
        SceneManager& scenes
    );

    void handleTouch(
        const IInputBackend& input,
        SceneManager& scenes
    );

    void handleAnalog(const IInputBackend& input);

    int itemIndexAt(float x, float y) const;

    Texture2D* arcsBg_ = nullptr;
    Texture2D* arcsFg_ = nullptr;
    Texture2D* banners_ = nullptr;
    Texture2D* dashboard_ = nullptr;
    Texture2D* aboutQr_ = nullptr;
    Texture2D* badges_ = nullptr;
    Texture2D* uiButtons_ = nullptr;

    BitmapFont* font_ = nullptr;

    Sprite title_;
    Sprite titleSigns_;

    std::array<DashboardButton, 6> buttons_;

    int selected_ = 0;
    int pressed_ = -1;

    bool analogLatched_ = false;

    float arcsBgOffset_ = 0.0f;
    float arcsFgOffset_ = 0.0f;
    float signsTime_ = 0.0f;

    float statusTimer_ = 0.0f;
    std::string statusText_;
    int panel_ = 0;
    int badgePage_ = 0;
    int rankingPage_ = 0;
    int settingsSelection_ = 0;
    bool onlineLoading_ = false;
    OnlineLeaderboardRequest onlineRequest_;
    OnlineLeaderboardResult onlineResults_;

    void updateButtonLabels();
};

} // namespace pdv
