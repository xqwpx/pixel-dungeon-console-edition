#pragma once

#include <array>

#include "game/actors/Actor.h"
#include "game/levels/LevelConstants.h"

namespace pdv {

class Dungeon;

enum class BlobKind {
    Fire,
    ToxicGas,
    ParalyticGas,
    ConfusionGas,
    WaterHealth,
    WaterAwareness,
    WaterTransmutation,
    Alchemy,
    Web,
    Regrowth,
    Foliage,
    SacrificialFire
};

class Blob : public Actor {
public:
    Blob();
    ~Blob() override = default;

    void bind(Dungeon* dungeon) {
        dungeon_ = dungeon;
    }

    Dungeon* dungeon() const {
        return dungeon_;
    }

    bool act() override;

    virtual BlobKind kind() const = 0;

    virtual void seed(int cell, int amount);
    void clear(int cell);

    int volume() const { return volume_; }

    int valueAt(int cell) const {
        return cell >= 0 && cell < LevelLength
            ? cur_[static_cast<std::size_t>(cell)]
            : 0;
    }

    void setValueAt(int cell, int value);

    const std::array<int, LevelLength>& cur() const {
        return cur_;
    }

protected:
    virtual void evolve();

    std::array<int, LevelLength> cur_{};
    std::array<int, LevelLength> off_{};

    int volume_ = 0;
    Dungeon* dungeon_ = nullptr;
};

} // namespace pdv
