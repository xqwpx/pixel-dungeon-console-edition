#pragma once

#include "core/GameSession.h"
#include "render/Sprite.h"

namespace pdv {

class BitmapFont;
class Renderer2D;
class Texture2D;

class HeroShield {
public:
    void initialize(
        Texture2D* avatarsTexture,
        BitmapFont* font,
        HeroClass heroClass
    );

    void setRect(float x, float y, float width, float height);

    bool contains(float x, float y) const;

    void update(float dt, bool selected);
    void draw(Renderer2D& renderer, bool selected) const;

    HeroClass heroClass() const { return heroClass_; }

private:
    BitmapFont* font_ = nullptr;
    HeroClass heroClass_ = HeroClass::Warrior;

    Sprite avatar_;

    float x_ = 0.0f;
    float y_ = 0.0f;
    float width_ = 0.0f;
    float height_ = 0.0f;

    float brightness_ = 0.60f;
};

} // namespace pdv
