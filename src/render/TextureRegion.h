#pragma once

namespace pdv {

struct TextureRegion {
    float x = 0.0f;
    float y = 0.0f;
    float w = 0.0f;
    float h = 0.0f;

    TextureRegion() = default;

    TextureRegion(float px, float py, float pw, float ph)
        : x(px), y(py), w(pw), h(ph) {}
};

} // namespace pdv
