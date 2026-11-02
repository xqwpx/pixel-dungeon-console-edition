#include "render/Sprite.h"

#include "assets/Texture2D.h"
#include "render/Renderer2D.h"

namespace pdv {

Sprite::Sprite(Texture2D* texture, const TextureRegion& region)
    : texture_(texture),
      region_(region),
      width_(region.w),
      height_(region.h) {
}

void Sprite::draw(Renderer2D& renderer) const {
    if (texture_ == nullptr) {
        return;
    }

    renderer.texture(
        *texture_,
        x_,
        y_,
        width_,
        height_,
        region_,
        alpha_,
        brightness_
    );
}

} // namespace pdv
