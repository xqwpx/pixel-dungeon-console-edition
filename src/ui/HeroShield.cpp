#include "ui/HeroShield.h"

#include "render/BitmapFont.h"
#include "render/Renderer2D.h"
#include "core/Localization.h"

#include <algorithm>

namespace pdv {

namespace {
constexpr float AvatarFrameWidth = 24.0f;
constexpr float AvatarFrameHeight = 28.0f;
constexpr float AvatarScale = 1.0f;
constexpr float MinimumBrightness = 0.78f;

const char* heroAbility(HeroClass heroClass) {
    switch (heroClass) {
        case HeroClass::Warrior:  return "Strength and armour";
        case HeroClass::Mage:     return "Fast wand recharge";
        case HeroClass::Rogue:    return "Cloak of Shadows";
        case HeroClass::Huntress: return "Ranged attacks";
    }
    return "";
}
}

void HeroShield::initialize(
    Texture2D* avatarsTexture,
    BitmapFont* font,
    HeroClass heroClass
) {
    font_ = font;
    heroClass_ = heroClass;

    const int index = static_cast<int>(heroClass_);

    avatar_ = Sprite(
        avatarsTexture,
        TextureRegion(
            static_cast<float>(index) * AvatarFrameWidth,
            0.0f,
            AvatarFrameWidth,
            AvatarFrameHeight
        )
    );

    avatar_.setSize(
        AvatarFrameWidth * AvatarScale,
        AvatarFrameHeight * AvatarScale
    );
}

void HeroShield::setRect(
    float x,
    float y,
    float width,
    float height
) {
    x_ = x;
    y_ = y;
    width_ = width;
    height_ = height;

    const float avatarW = AvatarFrameWidth * AvatarScale;
    const float avatarH = AvatarFrameHeight * AvatarScale;

    avatar_.setPosition(
        x_ + 7.0f,
        y_ + (height_ - avatarH) * 0.5f
    );
}

bool HeroShield::contains(float x, float y) const {
    return x >= x_ &&
           y >= y_ &&
           x < x_ + width_ &&
           y < y_ + height_;
}

void HeroShield::update(float dt, bool selected) {
    const float target = selected ? 1.0f : MinimumBrightness;
    const float speed = 4.0f;

    if (brightness_ < target) {
        brightness_ = std::min(target, brightness_ + dt * speed);
    } else if (brightness_ > target) {
        brightness_ = std::max(target, brightness_ - dt * speed);
    }
}

void HeroShield::draw(Renderer2D& renderer, bool selected) const {
    if (selected) {
        renderer.rect(
            x_ + 2.0f,
            y_ + 2.0f,
            width_ - 4.0f,
            height_ - 4.0f,
            0xFFFF8818
        );

        renderer.outlineRect(
            x_ + 1.0f,
            y_ + 1.0f,
            width_ - 2.0f,
            height_ - 2.0f,
            1.0f,
            0xFFFF88D0
        );
    }

    Sprite avatar = avatar_;
    avatar.setBrightness(brightness_);
    avatar.draw(renderer);

    if (font_ != nullptr) {
        const float textX = x_ + AvatarFrameWidth * AvatarScale + 14.0f;
        const float brightness = selected ? 1.0f : 0.90f;
        font_->draw(
            renderer,
            Localization::text(heroClassName(heroClass_)),
            textX,
            y_ + 5.0f,
            1.0f,
            brightness,
            1.0f
        );
        font_->draw(
            renderer,
            Localization::text(heroAbility(heroClass_)),
            textX,
            y_ + 20.0f,
            1.0f,
            selected ? 1.0f : 0.90f,
            1.0f
        );
    }
}

} // namespace pdv
