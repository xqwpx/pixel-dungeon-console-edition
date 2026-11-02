#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <array>

#include "core/GameSession.h"
#include "core/Localization.h"
#include "core/StoragePaths.h"
#include "game/actors/Actor.h"
#include "game/actors/Hero.h"
#include "game/actors/buffs/Hunger.h"
#include "game/levels/SewerLevel.h"
#include "game/items/Item.h"

namespace pdv {

class Char;
class Mob;
class SpecialMob;
enum class SpecialMobKind;
class Item;
class Scroll;

struct CombatActionResult {
    bool acted = false;
    bool attacked = false;
    bool hit = false;
    bool targetDied = false;

    int attackerHpBefore = 0;
    int attackerHpAfter = 0;

    int targetHpBefore = 0;
    int targetHpAfter = 0;
};

class Dungeon {
public:
    ~Dungeon();
    bool startNew(
        HeroClass heroClass,
        uint32_t seed
    );
    bool descend();
    // Return route after finding the Amulet. The original game lets the hero
    // climb all the way back to floor 1.
    bool ascend();
    // Developer-cheat transition. Floors are generated deterministically from
    // the active run seed, while the hero and their inventory stay intact.
    bool teleportToDepth(int targetDepth);
    bool enterDeveloperTestRoom();
    bool save(const std::string& path = StoragePaths::saveGame()) const;
    bool load(const std::string& path = StoragePaths::saveGame());
    static bool saveExists(const std::string& path = StoragePaths::saveGame());
    bool leaveBones(const std::string& path = StoragePaths::bones()) const;
    std::unique_ptr<Item> takeBones(int depth,const std::string& path = StoragePaths::bones()) const;

    bool started() const { return started_; }
    bool developerTestRoom() const { return developerTestRoom_; }
    bool consumeAmuletPickup(){const bool picked=amuletJustPickedUp_;amuletJustPickedUp_=false;return picked;}
    bool won() const { return won_; }
    void completeRun();
    int depth() const { return depth_; }
    int gold() const { return gold_; }
    void addGold(int value);
    bool spendGold(int value){if(value<0||gold_<value)return false;gold_-=value;return true;}
    int questState(int index)const{return index>=0&&index<4?questStates_[static_cast<std::size_t>(index)]:0;}
    void setQuestState(int index,int state){if(index>=0&&index<4){if(index==3&&state>=2000){const int kind=(state-2000)/16;state=2000+kind*16+kind;}questStates_[static_cast<std::size_t>(index)]=state;if(index==0&&state==360)prepareGhostRewards();}}
    const Item* ghostReward(int choice) const { return choice >= 0 && choice < 2 ? ghostRewards_[static_cast<std::size_t>(choice)].get() : nullptr; }
    void setGhostRewards(std::unique_ptr<Item> weapon, std::unique_ptr<Item> armor) { ghostRewards_[0] = std::move(weapon); ghostRewards_[1] = std::move(armor); }
    std::unique_ptr<Item> takeGhostReward(int choice) { if (choice < 0 || choice >= 2) return nullptr; std::unique_ptr<Item> reward = std::move(ghostRewards_[static_cast<std::size_t>(choice)]); ghostRewards_[0].reset(); ghostRewards_[1].reset(); return reward; }
    bool blacksmithSpawned()const{return blacksmithSpawned_;}
    void restoreBlacksmithSpawned(bool value){blacksmithSpawned_=value;}
    int blacksmithQuestKind()const{return blacksmithQuestKind_;}
    void setBlacksmithQuestKind(int value){blacksmithQuestKind_=static_cast<uint8_t>(value>=1&&value<=2?value:0);}
    bool questNpcSpawned(int bit)const{return bit>=0&&bit<3&&(questNpcSpawnMask_&(1u<<bit))!=0;}
    void markQuestNpcSpawned(int bit){if(bit>=0&&bit<3)questNpcSpawnMask_|=static_cast<uint8_t>(1u<<bit);}
    void restoreQuestNpcSpawnMask(uint8_t value){questNpcSpawnMask_=static_cast<uint8_t>(value&0x07u);}
    int pendingQuestReward() const;
    int pendingQuestRewardCount() const;
    std::array<std::string, 2> pendingQuestRewardNames() const;
    bool claimPendingQuestReward(int choice);
    bool canReforge(const Item& item) const;
    bool reforge(Item* first, Item* second);
    // Gameplay emits language-neutral English keys; presentation is localized
    // at this single boundary, including dynamically constructed messages.
    void postMessage(const std::string&message){lastMessage_=Localization::text(message);}
    std::string takeMessage(){std::string out=lastMessage_;lastMessage_.clear();return out;}

    bool posNeeded() const;
    bool souNeeded() const;
    bool soeNeeded() const;
    void notePotionOfStrength() { ++potionOfStrength_; }
    void noteScrollOfUpgrade() { ++scrollsOfUpgrade_; }
    void noteScrollOfEnchantment() { ++scrollsOfEnchantment_; }
    int potionOfStrengthCount() const { return potionOfStrength_; }
    int scrollOfUpgradeCount() const { return scrollsOfUpgrade_; }
    int scrollOfEnchantmentCount() const { return scrollsOfEnchantment_; }

    Level& level() { return level_; }
    const Level& level() const { return level_; }

    SewerLevel& sewerLevel() { return level_; }
    const SewerLevel& sewerLevel() const { return level_; }

    Hero& hero() { return hero_; }
    const Hero& hero() const { return hero_; }

    const std::array<bool, Level::Length>& visible() const {
        return visible_;
    }

    ActorScheduler& scheduler() { return scheduler_; }
    const ActorScheduler& scheduler() const { return scheduler_; }

    void observe();

    int findStep(int from, int to) const;

    int findPath(
        const Char& ch,
        int from,
        int to,
        const std::array<bool, Level::Length>& visible
    ) const;

    int flee(
        const Char& ch,
        int cur,
        int from,
        const std::array<bool, Level::Length>& visible
    ) const;

    bool moveHeroBy(int dx, int dy);
    bool moveHeroToAdjacent(int target);
    bool stepHeroToward(int target);
    bool waitTurn();
    int searchAdjacent();

    CombatActionResult heroAttackAt(int target);
    bool pickUpAtHero();
    bool openChestAt(int target);
    bool eatFood();
    bool eatFood(class Food* food);
    bool assignPrimaryQuickSlot(Item* item);
    bool unlockAt(int target);
    bool usePrimaryQuickSlot(int target);
    bool throwMissileAt(class MissileWeapon* weapon, int target);
    bool zapWandAt(class Wand* wand, int target);
    bool drinkPotion(class Potion* potion);
    bool throwPotionAt(class Potion* potion, int target);
    bool equipItem(Item* item);
    bool unequipItem(Item* item);
    bool dropItem(Item* item);
    bool throwItemAt(Item* item, int target);
    bool executeItemAction(Item* item, ItemAction action, int target = -1);

    bool readScroll(
        Scroll* scroll,
        Item* targetItem = nullptr
    );

    bool canTargetScroll(
        const Scroll& scroll,
        const Item& item
    ) const;

    bool consumeScrollOnCancel(
        Scroll* scroll
    );
    int firstVisibleMobCell() const;

    Hunger& hunger() { return hunger_; }
    const Hunger& hunger() const { return hunger_; }

    Mob* mobAt(int pos) const;
    SpecialMob* spawnSpecialMob(SpecialMobKind kind,int pos,float delay=0.0f);
    bool activateMimicAt(int pos, bool burning);
    bool relocateChar(Char& ch, int target);
    void noteMobAttack(int attackerCell) { lastMobAttackCell_ = attackerCell; ++mobAttackSerial_; }
    void recordKill(bool boss);
    void processQuestKill(const Mob& mob, const void* cause);
    // Called by the delayed test-arena actor after fifteen game seconds.
    void respawnDeveloperTestMob(int kind, int preferredCell);
    unsigned mobAttackSerial() const { return mobAttackSerial_; }
    int lastMobAttackCell() const { return lastMobAttackCell_; }

private:
    int search(bool intentional, bool spendTime);
    void passiveSearchFromDetection();
    void prepareGhostRewards();
    std::unique_ptr<Item> rollChestReward();
    bool spawnMimicAt(int pos);
    bool fallIntoChasm(int target);
    bool finishHeroMove(int target);
    void finishHeroTurn();

    bool started_ = false;
    bool won_ = false;
    bool runFinalized_ = false;
    int depth_ = 0;
    int gold_ = 0;
    int potionOfStrength_ = 0;
    int scrollsOfUpgrade_ = 0;
    int scrollsOfEnchantment_ = 0;
    uint32_t runSeed_ = 0;
    unsigned mobAttackSerial_ = 0;
    int lastMobAttackCell_ = -1;
    std::array<int,4> questStates_{};
    std::array<std::unique_ptr<Item>,2> ghostRewards_{};
    bool blacksmithSpawned_ = false;
    uint8_t blacksmithQuestKind_ = 0;
    uint8_t questNpcSpawnMask_ = 0;
    std::string lastMessage_;
    bool developerTestRoom_ = false;
    bool amuletJustPickedUp_ = false;
    std::vector<std::unique_ptr<Actor>> developerRespawnActors_;

    SewerLevel level_;
    Hero hero_;
    Hunger hunger_;

    std::array<bool, Level::Length> visible_{};

    ActorScheduler scheduler_;
};

} // namespace pdv
