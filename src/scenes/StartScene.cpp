#include "scenes/StartScene.h"

#include "assets/AssetCatalog.h"
#include "assets/TextureCache.h"
#include "audio/AudioSystem.h"
#include "core/SceneManager.h"
#include "core/Localization.h"
#include "game/Dungeon.h"
#include "engine/input/IInputBackend.h"
#include "render/BitmapFont.h"
#include "render/Renderer2D.h"
#include "ui/VitaButtonHints.h"

#include <cmath>


namespace pdv {
using VitaInput = IInputBackend;

namespace {

constexpr float ButtonHeight = 24.0f;
constexpr float SelectBannerWidth = 128.0f;
constexpr float SelectBannerHeight = 21.0f;
constexpr float ListLeft = 12.0f;
constexpr float ListTop = 39.0f;
constexpr float ListWidth = 176.0f;
constexpr float ListItemHeight = 44.0f;
constexpr float ActionWidth = 118.0f;
constexpr float ActionGap = 4.0f;
// Keep the actions clear of the controller hints along the lower edge.
constexpr float ActionY = 226.0f;

} // namespace

bool StartScene::initialize(
    TextureCache& textures,
    BitmapFont& font
) {
    banners_ = textures.get(Assets::Banners);
    avatars_ = textures.get(Assets::Avatars);
#if !defined(PD_PLATFORM_PS2)
    for (int i = 0; i < 4; ++i) {
        heroBackgrounds_[static_cast<std::size_t>(i)] =
            textures.get(Assets::HeroBackgrounds[static_cast<std::size_t>(i)]);
    }
#endif
    chrome_ = textures.get(Assets::Chrome);
    uiButtons_ = textures.get(Assets::UIButtonsPSVita);
    font_ = &font;

    if (banners_ == nullptr ||
        avatars_ == nullptr ||
        chrome_ == nullptr ||
        uiButtons_ == nullptr) {
        return false;
    }

#if !defined(PD_PLATFORM_PS2)
    for (Texture2D* background : heroBackgrounds_) {
        if (background == nullptr) {
            return false;
        }
    }
#endif

    // BannerSprites.Type.SELECT_YOUR_HERO:
    // banners.png: y 140..161 = 128x21.
    selectHeroBanner_ = Sprite(
        banners_,
        TextureRegion(
            0.0f,
            140.0f,
            SelectBannerWidth,
            SelectBannerHeight
        )
    );

    const float w = static_cast<float>(Renderer2D::LogicalWidth);
    const float h = static_cast<float>(Renderer2D::LogicalHeight);

    selectHeroBanner_.setPosition(
        ListLeft,
        14.0f
    );

    const HeroClass classes[4] = {
        HeroClass::Warrior,
        HeroClass::Mage,
        HeroClass::Rogue,
        HeroClass::Huntress
    };

    for (int i = 0; i < 4; ++i) {
        heroes_[static_cast<std::size_t>(i)].initialize(
            avatars_,
            font_,
            classes[i]
        );

        heroes_[static_cast<std::size_t>(i)].setRect(
            ListLeft,
            ListTop + static_cast<float>(i) * ListItemHeight,
            ListWidth,
            ListItemHeight
        );
    }

    newGameButton_.initialize(
        chrome_,
        font_,
        "New Game"
    );

    newGameButton_.setRect(
        (w - (ActionWidth * 2.0f + ActionGap)) * 0.5f,
        ActionY,
        ActionWidth,
        ButtonHeight
    );

    continueButton_.initialize(chrome_, font_, "Continue");
    continueButton_.setRect(
        newGameButton_.x() + ActionWidth + ActionGap,
        ActionY,
        ActionWidth,
        ButtonHeight
    );

    return true;
}

void StartScene::shutdown() {
    banners_ = nullptr;
    avatars_ = nullptr;
    heroBackgrounds_.fill(nullptr);
    chrome_ = nullptr;
    uiButtons_ = nullptr;
    font_ = nullptr;
}

void StartScene::onEnter(const GameSession& session) {
    AudioSystem::instance().playMusic("theme.mp3");
    selectedHero_ = static_cast<int>(session.selectedHero);

    if (selectedHero_ < 0 || selectedHero_ >= 4) {
        selectedHero_ = 0;
    }

    touchedHero_ = -1;
    selectedAction_ = 0;
    analogLatched_ = false;
    saveAvailable_ = Dungeon::saveExists();
    newGameButton_.setLabel(Localization::text("New Game"));
    continueButton_.setLabel(Localization::text("Continue"));
}

void StartScene::onExit() {
    touchedHero_ = -1;
}

void StartScene::update(
    float dt,
    const VitaInput& input,
    SceneManager& scenes,
    GameSession& session
) {
    touchedHero_ = -1;

    handleButtons(input, scenes, session);
    handleAnalog(input);
    handleTouch(input, scenes, session);

    for (int i = 0; i < 4; ++i) {
        heroes_[static_cast<std::size_t>(i)].update(
            dt,
            i == selectedHero_
        );
    }
}

void StartScene::handleButtons(
    const VitaInput& input,
    SceneManager& scenes,
    GameSession& session
) {
    if (input.pressed(SCE_CTRL_UP)) {
        moveSelection(-1);
    }

    if (input.pressed(SCE_CTRL_DOWN)) {
        moveSelection(+1);
    }

    if (saveAvailable_ &&
        (input.pressed(SCE_CTRL_LEFT) || input.pressed(SCE_CTRL_RIGHT))) {
        selectedAction_ = selectedAction_ == 0 ? 1 : 0;
    }

    if (input.pressed(SCE_CTRL_CROSS)) {
        launchSelectedAction(scenes, session);
    }

    if (input.pressed(SCE_CTRL_CIRCLE)) {
        scenes.request(SceneId::Title);
    }
}

void StartScene::handleAnalog(const VitaInput& input) {
    const float x = input.leftX();
    const float y = input.leftY();

    if (!analogLatched_) {
        if (std::fabs(y) >= std::fabs(x) && y <= -0.65f) {
            moveSelection(-1);
            analogLatched_ = true;
        } else if (std::fabs(y) >= std::fabs(x) && y >= 0.65f) {
            moveSelection(+1);
            analogLatched_ = true;
        } else if (saveAvailable_ && (x <= -0.65f || x >= 0.65f)) {
            selectedAction_ = selectedAction_ == 0 ? 1 : 0;
            analogLatched_ = true;
        }
    } else if (std::fabs(x) < 0.30f && std::fabs(y) < 0.30f) {
        analogLatched_ = false;
    }
}

void StartScene::handleTouch(
    const VitaInput& input,
    SceneManager& scenes,
    GameSession& session
) {
    for (const auto& touch : input.touches()) {
        if (!touch.active || !touch.pressed) {
            continue;
        }

        const float x = touch.x * 0.5f;
        const float y = touch.y * 0.5f;

        const int hero = heroIndexAt(x, y);

        if (hero >= 0) {
            selectedHero_ = hero;
            touchedHero_ = hero;
            continue;
        }

        if (newGameButton_.contains(x, y)) {
            selectedAction_ = 0;
            launchSelectedAction(scenes, session);
            continue;
        }

        if (saveAvailable_ && continueButton_.contains(x, y)) {
            selectedAction_ = 1;
            launchSelectedAction(scenes, session);
            continue;
        }

    }
}

int StartScene::heroIndexAt(float x, float y) const {
    for (int i = 0; i < 4; ++i) {
        if (heroes_[static_cast<std::size_t>(i)].contains(x, y)) {
            return i;
        }
    }

    return -1;
}

void StartScene::moveSelection(int delta) {
    selectedHero_ += delta;

    while (selectedHero_ < 0) {
        selectedHero_ += 4;
    }

    while (selectedHero_ >= 4) {
        selectedHero_ -= 4;
    }
}

void StartScene::launchSelected(
    SceneManager& scenes,
    GameSession& session
) {
    session.selectedHero =
        static_cast<HeroClass>(selectedHero_);

    session.newGameRequested = true;
    session.continueRequested = false;

    session.runSeed += 1u;
    scenes.request(SceneId::Game);
}

void StartScene::launchSelectedAction(
    SceneManager& scenes,
    GameSession& session
) {
    if (selectedAction_ == 0) {
        launchSelected(scenes, session);
        return;
    }

    // A dark Continue button is disabled, not an alternative way to start a
    // new run. This also keeps the action safe if input arrives mid-refresh.
    if (!saveAvailable_) {
        return;
    }

    session.newGameRequested = false;
    session.continueRequested = true;
    scenes.request(SceneId::Game);
}

void StartScene::render(Renderer2D& renderer) {
    if (font_ == nullptr) {
        return;
    }

#if defined(PD_PLATFORM_PS2)
    // The original 960x544 hero illustrations exceed the PS2 GS texture
    // budget. Keeping this screen texture-free prevents the Play transition
    // from stalling the renderer before a dungeon is started.
    renderer.rect(
        0.0f,
        0.0f,
        static_cast<float>(Renderer2D::LogicalWidth),
        static_cast<float>(Renderer2D::LogicalHeight),
        0x14202AFF
    );
#else
    renderer.fullTexture(
        *heroBackgrounds_[static_cast<std::size_t>(selectedHero_)],
        0.0f,
        0.0f,
        static_cast<float>(Renderer2D::LogicalWidth),
        static_cast<float>(Renderer2D::LogicalHeight),
        1.0f
    );
#endif

    // Darken only the list edge; the selected hero's artwork remains readable.
    renderer.rect(0.0f, 0.0f, 196.0f, static_cast<float>(Renderer2D::LogicalHeight), 0x080B12B3);

    selectHeroBanner_.draw(renderer);

    for (int i = 0; i < 4; ++i) {
        heroes_[static_cast<std::size_t>(i)].draw(
            renderer,
            i == selectedHero_
        );
    }

    newGameButton_.draw(
        renderer,
        selectedAction_ == 0,
        false
    );

    continueButton_.draw(renderer, selectedAction_ == 1, false, saveAvailable_);

    if (uiButtons_ != nullptr) {
        drawVitaButtonHint(renderer, *font_, *uiButtons_, {
            {VitaButtonIcon::DPad, " " + Localization::text("hero")},
            {VitaButtonIcon::Cross, " " + Localization::text("Select")},
            {VitaButtonIcon::Circle, " " + Localization::text("Back")}
        }, 244.0f, 258.0f, 0.50f, 0.50f);
    }
}

} // namespace pdv
