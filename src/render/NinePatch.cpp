#include "render/NinePatch.h"

#include "assets/Texture2D.h"
#include "render/Renderer2D.h"

#include <algorithm>

namespace pdv {

NinePatch::NinePatch(
    Texture2D* texture,
    const TextureRegion& region,
    float left,
    float top,
    float right,
    float bottom
) {
    configure(texture, region, left, top, right, bottom);
}

void NinePatch::configure(
    Texture2D* texture,
    const TextureRegion& region,
    float left,
    float top,
    float right,
    float bottom
) {
    texture_ = texture;
    region_ = region;

    left_ = left;
    top_ = top;
    right_ = right;
    bottom_ = bottom;
}

void NinePatch::draw(
    Renderer2D& renderer,
    float x,
    float y,
    float width,
    float height,
    float brightness,
    float alpha
) const {
    if (texture_ == nullptr || width <= 0.0f || height <= 0.0f) {
        return;
    }

    const float dstLeft = std::min(left_, width * 0.5f);
    const float dstRight = std::min(right_, width - dstLeft);

    const float dstTop = std::min(top_, height * 0.5f);
    const float dstBottom = std::min(bottom_, height - dstTop);

    const float srcCenterW =
        std::max(0.0f, region_.w - left_ - right_);

    const float srcCenterH =
        std::max(0.0f, region_.h - top_ - bottom_);

    const float dstCenterW =
        std::max(0.0f, width - dstLeft - dstRight);

    const float dstCenterH =
        std::max(0.0f, height - dstTop - dstBottom);

    const float sx[4] = {
        region_.x,
        region_.x + left_,
        region_.x + left_ + srcCenterW,
        region_.x + region_.w
    };

    const float sy[4] = {
        region_.y,
        region_.y + top_,
        region_.y + top_ + srcCenterH,
        region_.y + region_.h
    };

    const float dx[4] = {
        x,
        x + dstLeft,
        x + dstLeft + dstCenterW,
        x + width
    };

    const float dy[4] = {
        y,
        y + dstTop,
        y + dstTop + dstCenterH,
        y + height
    };

    for (int row = 0; row < 3; ++row) {
        for (int col = 0; col < 3; ++col) {
            const float srcW = sx[col + 1] - sx[col];
            const float srcH = sy[row + 1] - sy[row];
            const float dstW = dx[col + 1] - dx[col];
            const float dstH = dy[row + 1] - dy[row];

            if (srcW <= 0.0f || srcH <= 0.0f ||
                dstW <= 0.0f || dstH <= 0.0f) {
                continue;
            }

            renderer.texture(
                *texture_,
                dx[col],
                dy[row],
                dstW,
                dstH,
                TextureRegion(
                    sx[col],
                    sy[row],
                    srcW,
                    srcH
                ),
                alpha,
                brightness
            );
        }
    }
}

} // namespace pdv
