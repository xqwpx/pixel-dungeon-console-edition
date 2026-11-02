#pragma once

namespace pdv {

enum class SceneId {
    Title,
    Start,
    Game
};

class SceneManager {
public:
    SceneId current() const { return current_; }

    void request(SceneId scene);
    bool hasPending() const { return hasPending_; }

    SceneId consumePending();

private:
    SceneId current_ = SceneId::Title;
    SceneId pending_ = SceneId::Title;
    bool hasPending_ = false;
};

} // namespace pdv
