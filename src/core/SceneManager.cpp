#include "core/SceneManager.h"

namespace pdv {

void SceneManager::request(SceneId scene) {
    pending_ = scene;
    hasPending_ = true;
}

SceneId SceneManager::consumePending() {
    if (hasPending_) {
        current_ = pending_;
        hasPending_ = false;
    }

    return current_;
}

} // namespace pdv
