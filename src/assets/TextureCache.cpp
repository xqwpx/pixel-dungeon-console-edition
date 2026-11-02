#include "assets/TextureCache.h"

namespace pdv {

Texture2D* TextureCache::get(const std::string& path) {
    const auto found = textures_.find(path);
    if (found != textures_.end()) {
        return found->second.get();
    }

    std::unique_ptr<Texture2D> texture(new Texture2D());
    if (!texture->loadPng(path)) {
        lastError_ = texture->error();
        return nullptr;
    }

    Texture2D* raw = texture.get();
    textures_.emplace(path, std::move(texture));
    lastError_.clear();
    return raw;
}

void TextureCache::clear() {
    textures_.clear();
    lastError_.clear();
}

} // namespace pdv
