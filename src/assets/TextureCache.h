#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include "assets/Texture2D.h"

namespace pdv {

class TextureCache {
public:
    Texture2D* get(const std::string& path);
    const std::string& lastError() const { return lastError_; }
    void clear();

private:
    std::unordered_map<std::string, std::unique_ptr<Texture2D>> textures_;
    std::string lastError_;
};

} // namespace pdv
