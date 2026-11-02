#pragma once

#include <cstdint>
#include <memory>
#include <vector>

#include "game/levels/Level.h"
#include "game/levels/Room.h"
#include "game/levels/Terrain.h"

namespace pdv {

class RegularLevel : public Level {
public:
    bool create(int depth, uint32_t seed);

    int depth() const {
        return depth_;
    }

    const std::vector<std::unique_ptr<Room>>& rooms() const {
        return rooms_;
    }

    int secretDoors() const {
        return secretDoors_;
    }

    int nMobs() const;
    int randomRespawnCell();
    int pitCell() const;
    bool isWeakFloorCell(int cell) const;
    int randomDestination() override;
    void press(int cell, Char* ch) override;
    void sealBossEntrance();
    void unsealBossEntrance();
    bool bossEntranceSealed() const { return sealedEntrance_ != 0 || (bossArenaDoor_ != 0 && cell(bossArenaDoor_) == Terrain::LOCKED_DOOR); }
    bool weakFloorCreated()const{return weakFloorCreated_;}
    void restoreWeakFloorCreated(bool value){weakFloorCreated_=value;}
    bool bossArenaActivated()const{return bossArenaActivated_;}
    void restoreBossArenaActivated(bool value){bossArenaActivated_=value;}

protected:
    virtual int water() const = 0;
    virtual int grass() const = 0;
    virtual void decorate() = 0;

    bool build();

    virtual int maxRoomSize() const {
        return 9;
    }

    virtual int nTraps() const;

private:
    bool buildBossArena();
    bool buildFallbackLevel();
    bool buildLastShop();
    void initRooms();
    void split(
        const Rect& rect,
        std::vector<Rect>& out
    );

    void assignRoomType();

    void paint();
    void placeDoors(Room& room);
    void paintRoom(Room& room);
    bool joinRooms(Room& room, Room& other);
    void paintDoors(Room& room);

    void paintWater();
    void paintGrass();
    void placeTraps();

    void createMobs();
    void createItems();
    int randomDropCell();
    bool activateYogArena();
    bool activateKingArena();
    bool activateTenguArena();
    bool activateDM300Arena();

    Room* randomRoom(
        Room::Type type,
        int tries
    );

    int depth_ = 1;
    int secretDoors_ = 0;

    uint32_t seed_ = 0;

    std::vector<std::unique_ptr<Room>> rooms_;

    Room* roomEntrance_ = nullptr;
    Room* roomExit_ = nullptr;
    int sealedEntrance_ = 0;
    int bossEntranceCell_ = 0;
    int bossArenaDoor_ = 0;
    bool weakFloorCreated_ = false;
    bool bossArenaActivated_ = false;
    bool pitRoomNeeded_ = false;
};

} // namespace pdv
