#pragma once

#include "render/TextureRegion.h"

namespace pdv {

class Renderer2D;
class Texture2D;

class NinePatch {
public:
    NinePatch() = default;

    NinePatch(
        Texture2D* texture,
        const TextureRegion& region,
        float left,
        float top,
        float right,
        float bottom
    );

    void configure(
        Texture2D* texture,
        const TextureRegion& region,
        float left,
        float top,
        float right,
        float bottom
    );

    void draw(
        Renderer2D& renderer,
        float x,
        float y,
        float width,
        float height,
        float brightness = 1.0f,
        float alpha = 1.0f
    ) const;

private:
    Texture2D* texture_ = nullptr;
    TextureRegion region_;

    float left_ = 0.0f;
    float top_ = 0.0f;
    float right_ = 0.0f;
    float bottom_ = 0.0f;
};

} // namespace pdv
