#pragma once

#include <array>
#include <memory>
#include <vector>
#include <unordered_map>

namespace pdv {

class Mob;
class Heap;
class Item;
class Char;
class Dungeon;
class Blob;
enum class BlobKind;
enum class SeedKind;

class Level {
public:
    enum class Feeling {
        None,
        Chasm,
        Water,
        Grass
    };

    static constexpr int Width = 32;
    static constexpr int Height = 32;
    static constexpr int Length = Width * Height;
    static constexpr int ViewDistance = 8;

    static const std::array<int, 4> Neighbours4;
    static const std::array<int, 8> Neighbours8;
    static const std::array<int, 9> Neighbours9;

    Level();
    virtual ~Level();

    void createCoreParityTestMap();
    // An isolated developer arena. It is entered only through Dungeon's
    // cheat command and is never part of normal level generation.
    void createDeveloperTestMap();

    void beginGeneration(Feeling feeling);
    void finishGeneration();

    void buildFlagMaps();
    void cleanWalls();

    bool inBounds(int x, int y) const;
    bool inBoundsCell(int c) const {
        return c >= 0 && c < Length;
    }

    int cell(int x, int y) const;
    int cell(int p) const;

    void setRaw(int x, int y, int terrain);
    void setRaw(int p, int terrain);

    bool passable(int p) const {
        return inBoundsCell(p) && passable_[p];
    }

    bool avoid(int p) const {
        return inBoundsCell(p) && avoid_[p];
    }

    bool losBlocking(int p) const {
        return inBoundsCell(p) && losBlocking_[p];
    }

    bool flamable(int p) const {
        return inBoundsCell(p) && flamable_[p];
    }

    bool water(int p) const {
        return inBoundsCell(p) && water_[p];
    }

    bool pit(int p) const {
        return inBoundsCell(p) && pit_[p];
    }

    const std::array<bool, Length>& solidMap() const {
        return solid_;
    }

    const std::array<int, Length>& map() const {
        return map_;
    }

    std::array<int, Length>& mapMutable() {
        return map_;
    }

    const std::array<bool, Length>& passableMap() const {
        return passable_;
    }

    const std::array<bool, Length>& avoidMap() const {
        return avoid_;
    }

    const std::array<bool, Length>& losBlockingMap() const {
        return losBlocking_;
    }

    const std::array<bool, Length>& discoverableMap() const {
        return discoverable_;
    }

    const std::array<bool, Length>& fieldOfView() const {
        return fieldOfView_;
    }

    const std::array<bool, Length>& visited() const {
        return visited_;
    }

    const std::array<bool, Length>& mapped() const {
        return mapped_;
    }

    std::array<bool, Length>& mappedMutable() {
        return mapped_;
    }

    std::array<bool, Length>& visitedMutable() {
        return visited_;
    }

    void updateFieldOfView(
        int charPos,
        int viewDistance = ViewDistance
    );

    void updateFieldOfViewLimited(
        int charPos,
        int sense = 1
    );

    void updateFieldOfViewFor(Char& ch);

    int entrance() const {
        return entrance_;
    }

    int exit() const {
        return exit_;
    }

    void setEntrance(int value) {
        entrance_ = value;
    }

    void setExit(int value) {
        exit_ = value;
    }

    Feeling feeling() const {
        return feeling_;
    }

    virtual int viewDistance() const { return ViewDistance; }
    virtual int tunnelTile() const;
    virtual int randomDestination();

    const std::vector<std::unique_ptr<Mob>>& mobs() const {
        return mobs_;
    }

    std::vector<std::unique_ptr<Mob>>& mobsMutable() {
        return mobs_;
    }

    Mob* mobAt(int pos) const;
    void clearMobs();
    void addMob(std::unique_ptr<Mob> mob);
    int purgeDeadMobs();

    Heap* heapAt(int pos) const;
    Heap* drop(std::unique_ptr<Item> item, int pos);
    void clearHeaps();
    void removeEmptyHeap(int pos);
    const std::unordered_map<int, std::unique_ptr<Heap>>& heaps() const { return heaps_; }
    const std::unordered_map<int, SeedKind>& plants() const { return plants_; }
    bool plant(SeedKind kind,int pos);
    bool activatePlant(int pos,Char* ch);
    void clearPlants(){plants_.clear();}

    void addItemToSpawn(std::unique_ptr<Item> item);
    std::unique_ptr<Item> itemToSpawnAsPrize();
    void clearItemsToSpawn();
    const std::vector<std::unique_ptr<Item>>& itemsToSpawn() const { return itemsToSpawn_; }

    void bindDungeon(Dungeon* dungeon) {
        dungeon_ = dungeon;
    }

    Dungeon* dungeonContext() const {
        return dungeon_;
    }

    Blob* blob(BlobKind kind) const;
    Blob* seedBlob(
        BlobKind kind,
        int cell,
        int amount
    );
    void clearBlobs();
    const std::vector<std::unique_ptr<Blob>>& blobs() const { return blobs_; }

    void destroy(int pos);

    virtual void press(int cell, Char* ch);
    void mobPress(Mob& mob);

    static int distance(int a, int b);
    static bool adjacent(int a, int b);

protected:
    std::array<int, Length> map_{};
    std::vector<std::unique_ptr<Mob>> mobs_;
    std::unordered_map<int, std::unique_ptr<Heap>> heaps_;
    std::unordered_map<int, SeedKind> plants_;
    std::vector<std::unique_ptr<Blob>> blobs_;
    std::vector<std::unique_ptr<Item>> itemsToSpawn_;

private:
    int getWaterTile(int pos) const;

    std::array<bool, Length> visited_{};
    std::array<bool, Length> mapped_{};
    std::array<bool, Length> fieldOfView_{};

    std::array<bool, Length> passable_{};
    std::array<bool, Length> losBlocking_{};
    std::array<bool, Length> flamable_{};
    std::array<bool, Length> secret_{};
    std::array<bool, Length> solid_{};
    std::array<bool, Length> avoid_{};
    std::array<bool, Length> water_{};
    std::array<bool, Length> pit_{};
    std::array<bool, Length> discoverable_{};

    int entrance_ = 0;
    int exit_ = 0;

    Feeling feeling_ = Feeling::None;
    Dungeon* dungeon_ = nullptr;
};

} // namespace pdv
