#pragma once

#include "render/TextureRegion.h"

namespace pdv {

class Renderer2D;
class Texture2D;

class Sprite {
public:
    Sprite() = default;
    Sprite(Texture2D* texture, const TextureRegion& region);

    void setTexture(Texture2D* texture) { texture_ = texture; }
    void setRegion(const TextureRegion& region) { region_ = region; }

    void setPosition(float x, float y) {
        x_ = x;
        y_ = y;
    }

    void setSize(float w, float h) {
        width_ = w;
        height_ = h;
    }

    void setAlpha(float alpha) { alpha_ = alpha; }
    void setBrightness(float brightness) { brightness_ = brightness; }

    float x() const { return x_; }
    float y() const { return y_; }
    float width() const { return width_; }
    float height() const { return height_; }

    void draw(Renderer2D& renderer) const;

private:
    Texture2D* texture_ = nullptr;
    TextureRegion region_;

    float x_ = 0.0f;
    float y_ = 0.0f;
    float width_ = 0.0f;
    float height_ = 0.0f;

    float alpha_ = 1.0f;
    float brightness_ = 1.0f;
};

} // namespace pdv
