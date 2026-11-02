#include "game/Dungeon.h"
#include "core/StringUtils.h"
#if defined(PD_PLATFORM_PS1)
#include "platform/ps1/PS1MemoryCard.h"
#endif

#include "game/actors/Char.h"
#include "game/actors/buffs/Buff.h"
#include "game/actors/buffs/Regeneration.h"
#include "game/actors/buffs/Burning.h"
#include "game/actors/buffs/Frost.h"
#include "game/actors/mobs/Mob.h"
#include "game/actors/mobs/Mimic.h"
#include "game/actors/mobs/SpecialMob.h"
#include "game/actors/mobs/ZoneMob.h"
#include "game/actors/mobs/Goo.h"
#include "game/actors/mobs/Mimic.h"
#include "game/actors/mobs/Rat.h"
#include "game/actors/mobs/Albino.h"
#include "game/actors/mobs/Gnoll.h"
#include "game/actors/mobs/Crab.h"
#include "game/actors/mobs/Swarm.h"
#include "game/actors/mobs/Skeleton.h"
#include "game/actors/mobs/Thief.h"
#include "game/actors/mobs/Piranha.h"
#include "game/actors/mobs/npcs/DungeonNPC.h"
#include "game/items/Heap.h"
#include "game/items/Item.h"
#include "game/items/Gold.h"
#include "game/items/ConsumableCatalog.h"
#include "game/items/EquipmentCatalog.h"
#include "game/items/SpecialItems.h"
#include "game/items/food/Food.h"
#include "game/items/weapon/missiles/Dart.h"
#include "game/items/weapon/missiles/Boomerang.h"
#include "game/items/Generator.h"
#include "game/items/keys/IronKey.h"
#include "game/items/keys/GoldenKey.h"
#include "game/items/keys/SkeletonKey.h"
#include "game/items/bags/Keyring.h"
#include "game/items/bags/ScrollHolder.h"
#include "game/items/bags/SeedPouch.h"
#include "game/items/weapon/missiles/MissileWeapon.h"
#include "game/items/wands/Wand.h"
#include "game/items/wands/WandOfMagicMissile.h"
#include "game/items/potions/Potion.h"
#include "game/items/scrolls/Scroll.h"
#include "game/items/weapon/Weapon.h"
#include "game/items/armor/Armor.h"
#include "game/items/rings/Ring.h"
#include "game/actors/buffs/Blindness.h"
#include "game/actors/buffs/Fury.h"
#include "game/actors/buffs/SnipersMark.h"
#include "game/actors/buffs/Invisibility.h"
#include "game/actors/buffs/Shadows.h"
#include "game/actors/buffs/Terror.h"
#include "game/actors/buffs/Sleep.h"
#include "game/actors/buffs/Light.h"
#include "game/actors/buffs/Rage.h"
#include "game/actors/buffs/Weakness.h"
#include "game/actors/buffs/Burning.h"
#include "game/actors/buffs/Frost.h"
#include "game/actors/buffs/Poison.h"
#include "game/actors/buffs/Roots.h"
#include "game/actors/buffs/Paralysis.h"
#include "game/actors/buffs/Cripple.h"
#include "game/actors/buffs/Hunger.h"
#include "game/actors/buffs/Sleep.h"
#include "game/actors/mobs/npcs/MirrorImage.h"
#include "game/actors/mobs/npcs/DungeonNPC.h"
#include "game/mechanics/Ballistica.h"
#include "game/ui/QuickSlot.h"
#include "game/levels/Terrain.h"
#include "game/pathfinding/PathFinder.h"
#include "game/utils/Random.h"
#include "game/Progression.h"

#include <algorithm>
#include <cstdio>
#include <limits>
#include <cmath>

namespace pdv {

namespace {
enum TestMobKind { RatMob, AlbinoMob, GnollMob, CrabMob, GooMob, SwarmMob, SkeletonMob, ThiefMob, PiranhaMob, MimicMob, ZoneFirst = 100, BossFirst = 200, SpecialFirst = 300 };

std::unique_ptr<Mob> makeTestMob(int kind) {
    if (kind >= ZoneFirst && kind < ZoneFirst + 20) return std::unique_ptr<Mob>(new ZoneMob(static_cast<ZoneMobKind>(kind - ZoneFirst)));
    if (kind >= BossFirst && kind < BossFirst + 4) return std::unique_ptr<Mob>(new ZoneBoss(static_cast<ZoneBossKind>(kind - BossFirst)));
    if (kind >= SpecialFirst && kind < SpecialFirst + 5) return std::unique_ptr<Mob>(new SpecialMob(static_cast<SpecialMobKind>(kind - SpecialFirst), 10));
    switch (kind) {
    case RatMob: return std::unique_ptr<Mob>(new Rat()); case AlbinoMob: return std::unique_ptr<Mob>(new Albino());
    case GnollMob: return std::unique_ptr<Mob>(new Gnoll()); case CrabMob: return std::unique_ptr<Mob>(new Crab());
    case GooMob: return std::unique_ptr<Mob>(new Goo()); case SwarmMob: return std::unique_ptr<Mob>(new Swarm());
    case SkeletonMob: return std::unique_ptr<Mob>(new Skeleton()); case ThiefMob: return std::unique_ptr<Mob>(new Thief());
    case PiranhaMob: return std::unique_ptr<Mob>(new Piranha(10));
    case MimicMob: { std::unique_ptr<Mimic> mimic(new Mimic()); mimic->adjustStats(10); return std::move(mimic); }
    default: return nullptr;
    }
}

int testMobKind(const Mob& mob) {
    if (dynamic_cast<const Albino*>(&mob)) return AlbinoMob; if (dynamic_cast<const Rat*>(&mob)) return RatMob;
    if (dynamic_cast<const Gnoll*>(&mob)) return GnollMob; if (dynamic_cast<const Crab*>(&mob)) return CrabMob;
    if (dynamic_cast<const Goo*>(&mob)) return GooMob; if (dynamic_cast<const Swarm*>(&mob)) return SwarmMob;
    if (dynamic_cast<const Skeleton*>(&mob)) return SkeletonMob; if (dynamic_cast<const Thief*>(&mob)) return ThiefMob;
    if (dynamic_cast<const Piranha*>(&mob)) return PiranhaMob;
    if (dynamic_cast<const Mimic*>(&mob)) return MimicMob;
    if (const ZoneBoss* boss=dynamic_cast<const ZoneBoss*>(&mob)) return BossFirst+static_cast<int>(boss->kind());
    if (const ZoneMob* zone=dynamic_cast<const ZoneMob*>(&mob)) return ZoneFirst+static_cast<int>(zone->kind());
    if (const SpecialMob* special=dynamic_cast<const SpecialMob*>(&mob)) return SpecialFirst+static_cast<int>(special->kind());
    return -1;
}

class TestRespawnActor final : public Actor {
public:
    TestRespawnActor(Dungeon& dungeon,int kind,int cell):dungeon_(dungeon),kind_(kind),cell_(cell){}
    bool act() override { dungeon_.respawnDeveloperTestMob(kind_,cell_); deactivate(); next(); return true; }
private: Dungeon& dungeon_; int kind_,cell_;
};
}

Dungeon::~Dungeon(){ hero_.clearBuffs(); scheduler_.clear(); }

bool Dungeon::startNew(
    HeroClass heroClass,
    uint32_t seed
) {
    scheduler_.clear();
    developerRespawnActors_.clear();
    developerTestRoom_ = false;
    amuletJustPickedUp_ = false;
    visible_.fill(false);

    PathFinder::setMapSize(
        Level::Width,
        Level::Height
    );

    depth_ = 1;
    runSeed_ = seed;
    started_ = false;
    won_ = false;
    runFinalized_ = false;
    Progression::instance().beginRun(heroClass);
    gold_ = 0;
    potionOfStrength_ = 0;
    scrollsOfUpgrade_ = 0;
    scrollsOfEnchantment_ = 0;
    questStates_.fill(0);ghostRewards_[0].reset();ghostRewards_[1].reset();blacksmithSpawned_=false;blacksmithQuestKind_=0;questNpcSpawnMask_=0;lastMessage_.clear();
    Generator::reset();

    // Original Potion/Scroll ItemStatusHandler creates a random one-to-one
    // label/image permutation for a run. Reseed the gameplay RNG afterwards
    // so this presentation mapping does not change level generation.
    Random::seed(
        seed ^ 0x50A71001u
    );
    Potion::initColors();

    Random::seed(
        seed ^ 0x5C701101u
    );
    Scroll::initLabels();

    Random::seed(
        seed ^ 0xA71C6001u
    );
    Ring::initGems();

    Random::seed(
        seed ^ 0xA4D00001u
    );
    Wand::initAppearances();

    Random::seed(seed);

    level_.bindDungeon(this);

    if (!level_.create(depth_, seed)) {
        return false;
    }
    Progression::instance().noteDepth(depth_);

    hero_.initialize(
        heroClass,
        level_.entrance()
    );
    Progression::instance().noteHeroLevel(hero_.level());
    hero_.bindLevel(&level_);

    hunger_.reset();
    hunger_.bind(&hero_);
    hero_.bindHunger(&hunger_);

    // Original Hero.live(): persistent Regeneration and Hunger buffs.
    Buff::affect<Regeneration>(hero_);

    scheduler_.addDelayed(
        &hero_,
        -std::numeric_limits<float>::min()
    );
    scheduler_.add(&hunger_);

    for (auto& mob : level_.mobsMutable()) {
        if (!mob) {
            continue;
        }

        mob->bind(this);
        scheduler_.add(mob.get());
    }

    if (WandOfMagicMissile* wand = hero_.belongings().getItem<WandOfMagicMissile>()) {
        wand->activate(hero_, scheduler_);
    }

    scheduler_.process(&hero_);

    started_ = true;
    observe();

    return true;
}

bool Dungeon::descend() {
    if (!started_ || !hero_.isAlive() || depth_ >= 26 || hero_.pos != level_.exit()) {
        return false;
    }

    scheduler_.clear();
    visible_.fill(false);
    ++depth_;

    // A new floor starts on a fresh scheduler timeline. Persistent actors keep
    // their state, but must not carry absolute timestamps from the old floor.
    hero_.resetTime();
    hunger_.resetTime();
    for (Buff* buff : hero_.activeBuffs()) {
        if (buff) buff->resetTime();
    }

    if (WandOfMagicMissile* wand = hero_.belongings().getItem<WandOfMagicMissile>()) {
        wand->stopCharging();
    }

    const uint32_t floorSeed = runSeed_ ^
        (0x9E3779B9u * static_cast<uint32_t>(depth_));
    Random::seed(floorSeed);

    if (!level_.create(depth_, floorSeed)) {
        --depth_;
        return false;
    }
    Progression::instance().noteDepth(depth_);

    hero_.pos = level_.entrance();
    hero_.bindLevel(&level_);
    scheduler_.addDelayed(&hero_, -std::numeric_limits<float>::min());
    scheduler_.add(&hunger_);

    for (auto& mob : level_.mobsMutable()) {
        if (mob) {
            mob->bind(this);
            scheduler_.add(mob.get());
        }
    }

    if (WandOfMagicMissile* wand = hero_.belongings().getItem<WandOfMagicMissile>()) {
        wand->activate(hero_, scheduler_);
    }

    scheduler_.process(&hero_);
    observe();
    save();
    return true;
}

bool Dungeon::teleportToDepth(int targetDepth) {
    if (!started_ || !hero_.isAlive() || targetDepth < 1 ||
        targetDepth > 26 || targetDepth == depth_) {
        return false;
    }

    const int previousDepth = depth_;
    scheduler_.clear();
    developerRespawnActors_.clear();
    developerTestRoom_ = false;
    amuletJustPickedUp_ = false;
    visible_.fill(false);
    depth_ = targetDepth;

    // Like a normal descent, give every persistent actor a fresh timeline on
    // the newly generated map. This deliberately keeps hero stats, buffs,
    // hunger and belongings intact for cheat-driven testing.
    hero_.resetTime();
    hunger_.resetTime();
    for (Buff* buff : hero_.activeBuffs()) {
        if (buff) buff->resetTime();
    }

    if (WandOfMagicMissile* wand = hero_.belongings().getItem<WandOfMagicMissile>()) {
        wand->stopCharging();
    }

    const uint32_t floorSeed = runSeed_ ^
        (0x9E3779B9u * static_cast<uint32_t>(depth_));
    Random::seed(floorSeed);

    if (!level_.create(depth_, floorSeed)) {
        depth_ = previousDepth;
        return false;
    }
    // Levels are regenerated in this port when travelling back. Once the
    // Amulet is held, never recreate defeated bosses on the return journey.
    bool returningWithAmulet=false;
    for(const auto& item:hero_.belongings().backpack().items()){
        const SpecialItem* special=item?dynamic_cast<const SpecialItem*>(item.get()):nullptr;
        if(special&&special->kind()==SpecialKind::Amulet){returningWithAmulet=true;break;}
    }
    if(returningWithAmulet){
        auto& mobs=level_.mobsMutable();
        mobs.erase(std::remove_if(mobs.begin(),mobs.end(),[](const std::unique_ptr<Mob>& mob){return dynamic_cast<const ZoneBoss*>(mob.get())||dynamic_cast<const Goo*>(mob.get());}),mobs.end());
    }
    Progression::instance().noteDepth(depth_);

    hero_.pos = level_.entrance();
    hero_.bindLevel(&level_);
    scheduler_.addDelayed(&hero_, -std::numeric_limits<float>::min());
    scheduler_.add(&hunger_);

    for (auto& mob : level_.mobsMutable()) {
        if (mob) {
            mob->bind(this);
            scheduler_.add(mob.get());
        }
    }

    if (WandOfMagicMissile* wand = hero_.belongings().getItem<WandOfMagicMissile>()) {
        wand->activate(hero_, scheduler_);
    }

    scheduler_.process(&hero_);
    observe();
    save();
    return true;
}

bool Dungeon::ascend() {
    if (!started_ || !hero_.isAlive() || depth_ <= 1 || hero_.pos != level_.entrance()) return false;
    return teleportToDepth(depth_ - 1);
}

bool Dungeon::enterDeveloperTestRoom() {
    if (!started_ || !hero_.isAlive()) return false;
    scheduler_.clear();
    developerRespawnActors_.clear();
    visible_.fill(false);
    developerTestRoom_ = true;
    level_.createDeveloperTestMap();
    hero_.pos = level_.entrance(); hero_.bindLevel(&level_);
    scheduler_.addDelayed(&hero_, -std::numeric_limits<float>::min());
    scheduler_.add(&hunger_);

    std::vector<int> kinds={RatMob,AlbinoMob,GnollMob,CrabMob,GooMob,SwarmMob,SkeletonMob,ThiefMob,PiranhaMob,MimicMob};
    for(int i=0;i<20;++i) kinds.push_back(ZoneFirst+i);
    for(int i=0;i<4;++i) kinds.push_back(BossFirst+i);
    for(int i=0;i<5;++i) kinds.push_back(SpecialFirst+i);
    for(std::size_t i=0;i<kinds.size();++i){
        std::unique_ptr<Mob> mob=makeTestMob(kinds[i]); if(!mob) continue;
        const int x=3+static_cast<int>(i%13)*2, y=7+static_cast<int>(i/13)*3;
        mob->pos=x+y*Level::Width; mob->setState(Mob::State::Passive); mob->bind(this);
        Mob* raw=mob.get(); level_.addMob(std::move(mob)); scheduler_.add(raw);
    }
    // Non-hostile NPCs are included for visual and interaction testing.
    for(int i=0;i<6;++i){std::unique_ptr<Mob> npc(new DungeonNPC(static_cast<NPCKind>(i))); npc->pos=(3+i*4)+26*Level::Width; npc->setState(Mob::State::Passive); npc->bind(this); Mob* raw=npc.get(); level_.addMob(std::move(npc)); scheduler_.add(raw);}
    std::vector<std::unique_ptr<Item>> items=Generator::allImplementedItems(10);
    items.push_back(std::unique_ptr<Item>(new IronKey(10))); items.push_back(std::unique_ptr<Item>(new GoldenKey(10))); items.push_back(std::unique_ptr<Item>(new SkeletonKey(10)));
    items.push_back(std::unique_ptr<Item>(new Keyring())); items.push_back(std::unique_ptr<Item>(new ScrollHolder())); items.push_back(std::unique_ptr<Item>(new SeedPouch())); items.push_back(std::unique_ptr<Item>(new WandHolster()));
    for(int i=0;i<=static_cast<int>(SpecialKind::ChestPlaceholder);++i) items.push_back(std::unique_ptr<Item>(new SpecialItem(static_cast<SpecialKind>(i))));
    for(std::size_t i=0;i<items.size();++i) level_.drop(std::move(items[i]),(2+static_cast<int>(i%28))+(14+static_cast<int>(i/28))*Level::Width);
    scheduler_.process(&hero_); observe(); postMessage("Developer test room: passive mobs respawn 15 seconds after death.");
    return true;
}

void Dungeon::respawnDeveloperTestMob(int kind, int preferredCell) {
    if (!developerTestRoom_) return;
    std::unique_ptr<Mob> mob=makeTestMob(kind); if(!mob) return;
    int cell=preferredCell;
    if(!level_.inBoundsCell(cell)||scheduler_.findChar(cell)||(!level_.passable(cell)&&!level_.avoid(cell))) cell=level_.randomRespawnCell();
    mob->pos=cell; mob->setState(Mob::State::Passive); mob->bind(this); Mob* raw=mob.get(); level_.addMob(std::move(mob)); scheduler_.add(raw);
}

void Dungeon::observe() {
    if (!started_) {
        return;
    }

    if (!hero_.isAlive()) {
        visible_.fill(false);
        return;
    }

    level_.updateFieldOfViewFor(
        hero_
    );

    visible_ = level_.fieldOfView();

    auto& visited = level_.visitedMutable();

    for (int i = 0; i < Level::Length; ++i) {
        visited[i] = visited[i] || visible_[i];
    }
}

int Dungeon::findStep(int from, int to) const {
    return findPath(
        hero_,
        from,
        to,
        visible_
    );
}

int Dungeon::findPath(
    const Char& ch,
    int from,
    int to,
    const std::array<bool, Level::Length>& visible
) const {
    if (!level_.inBoundsCell(from) ||
        !level_.inBoundsCell(to)) {
        return -1;
    }

    if (Level::adjacent(from, to)) {
        return
            scheduler_.findChar(to) == nullptr &&
            (level_.passable(to) || (ch.flying && level_.avoid(to)))
            ? to
            : -1;
    }

    std::array<bool, Level::Length> pass = level_.passableMap();

    if (ch.flying) {
        for (int i = 0; i < Level::Length; ++i) {
            pass[i] = pass[i] || level_.avoid(i);
        }
    }

    for (Actor* actor : scheduler_.all()) {
        Char* character = dynamic_cast<Char*>(actor);

        if (!character ||
            !character->isAlive() ||
            !level_.inBoundsCell(character->pos)) {
            continue;
        }

        if (visible[static_cast<std::size_t>(character->pos)]) {
            pass[static_cast<std::size_t>(character->pos)] = false;
        }
    }

    return PathFinder::getStep(from, to, pass);
}

int Dungeon::flee(
    const Char& ch,
    int cur,
    int from,
    const std::array<bool, Level::Length>& visible
) const {
    std::array<bool, Level::Length> pass = level_.passableMap();

    if (ch.flying) {
        for (int i = 0; i < Level::Length; ++i) {
            pass[i] = pass[i] || level_.avoid(i);
        }
    }

    for (Actor* actor : scheduler_.all()) {
        Char* character = dynamic_cast<Char*>(actor);

        if (!character ||
            !character->isAlive() ||
            !level_.inBoundsCell(character->pos)) {
            continue;
        }

        if (visible[static_cast<std::size_t>(character->pos)]) {
            pass[static_cast<std::size_t>(character->pos)] = false;
        }
    }

    if (level_.inBoundsCell(cur)) {
        pass[static_cast<std::size_t>(cur)] = true;
    }

    PathFinder::buildDistanceMap(
        from,
        pass,
        Level::Length
    );

    const auto& distance = PathFinder::distance();

    int best = cur;
    int bestDistance =
        (cur >= 0 && cur < static_cast<int>(distance.size()))
        ? distance[static_cast<std::size_t>(cur)]
        : -1;

    for (int offset : Level::Neighbours8) {
        const int next = cur + offset;

        if (!level_.inBoundsCell(next) ||
            !pass[static_cast<std::size_t>(next)]) {
            continue;
        }

        const int value = distance[static_cast<std::size_t>(next)];

        if (value > bestDistance) {
            bestDistance = value;
            best = next;
        }
    }

    return best == cur ? -1 : best;
}

void Dungeon::finishHeroTurn() {
    observe();

    scheduler_.process(&hero_);

    // Dead mobs already left ActorScheduler in Char::destroy(); ownership is
    // removed only after all virtual calls return.
    level_.purgeDeadMobs();

    if(!hero_.isAlive()){
        SpecialItem* ankh=nullptr;for(Item* item:hero_.belongings().allItems()){SpecialItem* special=dynamic_cast<SpecialItem*>(item);if(special&&special->kind()==SpecialKind::Ankh){ankh=special;break;}}
        SpecialItem*fullVial=nullptr;for(Item*item:hero_.belongings().allItems())if(SpecialItem*special=dynamic_cast<SpecialItem*>(item))if(special->kind()==SpecialKind::DewVial&&special->quantity()>=10){fullVial=special;break;}
        if(fullVial){fullVial->quantity(0);hero_.HP=hero_.HT;hero_.paralysed=false;hero_.rooted=false;scheduler_.add(&hero_);}
        else if(ankh&&hero_.belongings().detach(ankh)){hero_.belongings().resurrect(depth_);gold_=0;hero_.clearExperience();hero_.HP=std::max(1,hero_.HT/4);hero_.paralysed=false;hero_.rooted=false;Progression::instance().noteAnkh();scheduler_.add(&hero_);}
        else if(!runFinalized_){leaveBones();Progression::instance().finishRun(false);runFinalized_=true;}
    }

    observe();
    save();
}

void Dungeon::passiveSearchFromDetection() {
    for (const Ring* ring : {hero_.belongings().ring1(), hero_.belongings().ring2()}) {
        if (ring && ring->detectsSecrets()) {
            search(false, false);
            return;
        }
    }
}

bool Dungeon::finishHeroMove(int target) {
    if (!hero_.isAlive()) {
        return false;
    }

    if (hero_.paralysed) {
        hero_.spendAndNext(Actor::TICK);
        finishHeroTurn();
        return false;
    }

    if (level_.inBoundsCell(target) &&
        (level_.cell(target) == Terrain::LOCKED_DOOR ||
         level_.cell(target) == Terrain::LOCKED_EXIT)) {
        return unlockAt(target);
    }

    if (level_.inBoundsCell(target) && level_.pit(target) && !hero_.flying) {
        return fallIntoChasm(target);
    }

    if (Mob* occupant = mobAt(target)) {
        if (!occupant->hostile()) {
            if(DungeonNPC*npc=dynamic_cast<DungeonNPC*>(occupant)){npc->interact(*this,hero_);hero_.spendAndNext(Actor::TICK);finishHeroTurn();return true;}
            const int oldHero = hero_.pos;
            const int oldMob = occupant->pos;
            scheduler_.freeCell(oldHero);
            scheduler_.freeCell(oldMob);
            hero_.pos = oldMob;
            occupant->pos = oldHero;
            scheduler_.occupyCell(&hero_);
            scheduler_.occupyCell(occupant);
            observe();
            passiveSearchFromDetection();
            hero_.spendMoveAndNext();
            finishHeroTurn();
            return true;
        }

        CombatActionResult result = heroAttackAt(target);
        return result.acted;
    }

    if (!hero_.moveTo(target, level_)) {
        return false;
    }

    if (!hero_.flying) {
        level_.press(
            hero_.pos,
            &hero_
        );
    }

    observe();
    passiveSearchFromDetection();
    hero_.spendMoveAndNext();
    finishHeroTurn();

    return true;
}

bool Dungeon::fallIntoChasm(int target) {
    if (!started_ || !hero_.isAlive() || depth_ >= 26 ||
        !level_.inBoundsCell(target) || !level_.pit(target)) return false;
    const RegularLevel* previous = dynamic_cast<const RegularLevel*>(&level_);
    const bool intoPit = previous && previous->isWeakFloorCell(target);

    scheduler_.clear();
    visible_.fill(false);
    ++depth_;

    hero_.resetTime();
    hunger_.resetTime();
    for (Buff* buff : hero_.activeBuffs()) if (buff) buff->resetTime();
    if (WandOfMagicMissile* wand = hero_.belongings().getItem<WandOfMagicMissile>()) wand->stopCharging();

    const uint32_t floorSeed = runSeed_ ^ (0x9E3779B9u * static_cast<uint32_t>(depth_));
    Random::seed(floorSeed);
    if (!level_.create(depth_, floorSeed)) {
        --depth_;
        return false;
    }
    Progression::instance().noteDepth(depth_);

    int landing = -1;
    if (intoPit) landing = level_.pitCell();
    if (landing < 0) landing = level_.randomRespawnCell();
    if (landing < 0) landing = level_.entrance();
    hero_.pos = landing;
    hero_.bindLevel(&level_);
    scheduler_.addDelayed(&hero_, -std::numeric_limits<float>::min());
    scheduler_.add(&hunger_);
    for (auto& mob : level_.mobsMutable()) if (mob) { mob->bind(this); scheduler_.add(mob.get()); }
    if (WandOfMagicMissile* wand = hero_.belongings().getItem<WandOfMagicMissile>()) wand->activate(hero_, scheduler_);

    Buff::prolong<Cripple>(hero_, Cripple::Duration);
    const int damage = Random::IntRange(std::max(1, hero_.HT / 3), std::max(1, hero_.HT / 2));
    if (hero_.HP <= damage) Progression::instance().noteDeath(Badge::DeathFalling);
    hero_.damage(damage, this);
    // finishHeroTurn performs the scheduler step.  Running it here as well
    // processed a freshly-created floor twice, leaving the scheduler with
    // stale actor state after a chasm transition.
    finishHeroTurn();
    return true;
}

bool Dungeon::moveHeroToAdjacent(int target) {
    return finishHeroMove(target);
}

bool Dungeon::moveHeroBy(int dx, int dy) {
    if (!hero_.isAlive() ||
        dx < -1 || dx > 1 ||
        dy < -1 || dy > 1 ||
        (dx == 0 && dy == 0)) {
        return false;
    }

    const int x = hero_.pos % Level::Width;
    const int y = hero_.pos / Level::Width;

    const int nx = x + dx;
    const int ny = y + dy;

    if (!level_.inBounds(nx, ny)) {
        return false;
    }

    return finishHeroMove(ny * Level::Width + nx);
}

bool Dungeon::stepHeroToward(int target) {
    if (!hero_.isAlive()) {
        return false;
    }

    if (hero_.pos == target) {
        return true;
    }

    if (Mob* mob = mobAt(target)) {
        if (Level::adjacent(hero_.pos, mob->pos)) {
            return heroAttackAt(target).acted;
        }
    }

    const int step = findStep(hero_.pos, target);

    if (step < 0 || step == hero_.pos) {
        return false;
    }

    return finishHeroMove(step);
}

bool Dungeon::waitTurn() {
    if (!started_ || !hero_.isAlive()) return false;
    hero_.spendAndNext(Actor::TICK);
    finishHeroTurn();
    return true;
}

CombatActionResult Dungeon::heroAttackAt(int target) {
    CombatActionResult result;

    if (!hero_.isAlive()) {
        return result;
    }

    if (hero_.paralysed) {
        hero_.spendAndNext(Actor::TICK);
        finishHeroTurn();
        return result;
    }

    Mob* mob = mobAt(target);

    if (!mob ||
        !mob->isAlive() ||
        !Level::adjacent(hero_.pos, mob->pos)) {
        return result;
    }

    if(!mob->hostile()){
        if(DungeonNPC*npc=dynamic_cast<DungeonNPC*>(mob))npc->interact(*this,hero_);
        else if(SpecialMob*special=dynamic_cast<SpecialMob*>(mob);special&&special->kind()==SpecialMobKind::Bee){
            const int oldHero=hero_.pos;
            const int oldBee=special->pos;
            scheduler_.freeCell(oldHero);
            scheduler_.freeCell(oldBee);
            hero_.pos=oldBee;
            special->pos=oldHero;
            scheduler_.occupyCell(&hero_);
            scheduler_.occupyCell(special);
        }
        hero_.spendAndNext(Actor::TICK);finishHeroTurn();result.acted=true;return result;
    }

    result.acted = true;
    result.attacked = true;

    result.attackerHpBefore = hero_.HP;
    result.targetHpBefore = mob->HP;

    result.hit = hero_.attack(*mob);

    result.targetHpAfter = std::max(0, mob->HP);
    result.targetDied = !mob->isAlive();

    hero_.spendAttackAndNext();
    finishHeroTurn();

    result.attackerHpAfter = std::max(0, hero_.HP);

    return result;
}

Mob* Dungeon::mobAt(int pos) const {
    return level_.mobAt(pos);
}

bool Dungeon::relocateChar(Char& ch,int target){
    if(!level_.inBoundsCell(target)||!(level_.passable(target)||(ch.flying&&level_.avoid(target)))||scheduler_.findChar(target))return false;
    scheduler_.freeCell(ch.pos);ch.pos=target;scheduler_.occupyCell(&ch);ch.bindLevel(&level_);if(!ch.flying)level_.press(target,&ch);observe();return true;
}

bool Dungeon::spawnMimicAt(int pos){
    Heap*heap=level_.heapAt(pos);if(!heap||heap->type!=Heap::Type::Mimic)return false;
    if(Char*occupant=scheduler_.findChar(pos)){std::vector<int>candidates;for(int offset:Level::Neighbours8){const int cell=pos+offset;if(level_.inBoundsCell(cell)&&level_.passable(cell)&&!scheduler_.findChar(cell))candidates.push_back(cell);}if(candidates.empty())return false;if(!relocateChar(*occupant,candidates[static_cast<std::size_t>(Random::Int(static_cast<int>(candidates.size())))]))return false;}
    std::unique_ptr<Mimic>mimic(new Mimic());Mimic*raw=mimic.get();mimic->adjustStats(depth_);mimic->pos=pos;mimic->bind(this);
    while(!heap->empty()){std::unique_ptr<Item>item=heap->pickUp();SpecialItem*marker=dynamic_cast<SpecialItem*>(item.get());if(marker&&marker->kind()==SpecialKind::ChestPlaceholder){item=rollChestReward();if(!item)item=std::unique_ptr<Item>(new Food());}mimic->addItem(std::move(item));}
    level_.removeEmptyHeap(pos);level_.addMob(std::move(mimic));scheduler_.addDelayed(raw,1.0f);raw->beckon(hero_.pos);return true;
}

bool Dungeon::activateMimicAt(int pos, bool burning) {
    if (!spawnMimicAt(pos)) return false;
    Mob* mimic = mobAt(pos);
    if (!mimic) return false;
    if (burning) {
        if (Burning* effect = Buff::affect<Burning>(*mimic)) effect->reignite(*mimic);
    } else {
        Buff::prolong<Frost>(*mimic, Frost::duration(*mimic) * Random::Float(1.0f, 1.5f));
    }
    return true;
}

SpecialMob*Dungeon::spawnSpecialMob(SpecialMobKind kind,int pos,float delay){if(!level_.inBoundsCell(pos)||(!level_.passable(pos)&&!level_.avoid(pos))||scheduler_.findChar(pos))return nullptr;std::unique_ptr<SpecialMob>mob(new SpecialMob(kind,depth_));SpecialMob*raw=mob.get();raw->pos=pos;raw->bind(this);level_.addMob(std::move(mob));scheduler_.addDelayed(raw,delay);return raw;}

void Dungeon::addGold(int value){if(value>0){gold_+=value;Progression::instance().noteGold(value);}}
void Dungeon::prepareGhostRewards(){
    if(!ghostRewards_[0])ghostRewards_[0]=Generator::random(Generator::Category::Weapon,depth_);
    if(!ghostRewards_[1])ghostRewards_[1]=Generator::random(Generator::Category::Armor,depth_);
}
void Dungeon::recordKill(bool boss){Progression::instance().noteKill(boss,hero_.subClass());}
int Dungeon::pendingQuestReward() const {
    if (questState(0) == 360) return 0;
    const int wandmaker = questState(1);
    const int wandmakerKinds = wandmaker - 1000;
    if (wandmakerKinds >= 0 && wandmakerKinds / 16 < 12 && wandmakerKinds % 16 < 12) return 1;
    const int imp = questState(3);
    const int impKinds = imp - 2000;
    if (impKinds >= 0 && impKinds / 16 < 11 && impKinds % 16 < 11) return 3;
    return -1;
}

int Dungeon::pendingQuestRewardCount() const {
    return pendingQuestReward() == 3 ? 1 : 2;
}

std::array<std::string, 2> Dungeon::pendingQuestRewardNames() const {
    const int quest = pendingQuestReward();
    if (quest == 0) {
        const Item* weapon = ghostReward(0);
        const Item* armor = ghostReward(1);
        return {{weapon ? weapon->displayName() : "Weapon", armor ? armor->displayName() : "Armor"}};
    }
    const int encoded = quest == 1 ? questState(1) - 1000 : quest == 3 ? questState(3) - 2000 : -1;
    if (encoded < 0) return {{"", ""}};
    const int first = encoded / 16;
    const int second = encoded % 16;
    if (quest == 1 && first < 12 && second < 12) {
        return {{CatalogWand(static_cast<WandKind>(first)).name(), CatalogWand(static_cast<WandKind>(second)).name()}};
    }
    if (quest == 3 && first < 11 && second < 11) {
        return {{CatalogRing(static_cast<RingKind>(first)).name(), CatalogRing(static_cast<RingKind>(second)).name()}};
    }
    return {{"", ""}};
}

bool Dungeon::claimPendingQuestReward(int choice) {
    if (choice < 0 || choice > 1) return false;
    const int quest = pendingQuestReward();
    if (quest == 0) {
        std::unique_ptr<Item> reward=takeGhostReward(choice);
        if (!reward) reward=Generator::random(choice==0?Generator::Category::Weapon:Generator::Category::Armor,depth_);
        if (!reward) return false;
        reward->identify();
        if (hero_.belongings().backpack().canCollect(*reward)) hero_.belongings().backpack().collect(std::move(reward));
        else level_.drop(std::move(reward),hero_.pos);
        setQuestState(0,2);
        Progression::instance().removeJournal(JournalFeature::Ghost,depth_);
        postMessage("The ghost rewards you and finds peace.");
        return true;
    }
    const int encoded = quest == 1 ? questState(1) - 1000 : quest == 3 ? questState(3) - 2000 : -1;
    const int kind = choice == 0 ? encoded / 16 : encoded % 16;
    std::unique_ptr<Item> reward;
    if (quest == 1 && kind >= 0 && kind < 12) reward.reset(new CatalogWand(static_cast<WandKind>(kind)));
    if (quest == 3 && kind >= 0 && kind < 11) reward.reset(new CatalogRing(static_cast<RingKind>(kind)));
    if (!reward) return false;
    reward->upgrade(quest == 1 ? 1 : 2);
    // Imp's promised ring is deliberately cursed after its +2 upgrade.
    if (quest == 3) reward->setCursed(true);
    reward->identify();
    if (hero_.belongings().backpack().canCollect(*reward)) hero_.belongings().backpack().collect(std::move(reward));
    else level_.drop(std::move(reward), hero_.pos);
    setQuestState(quest, 2);
    Progression::instance().removeJournal(quest == 1 ? JournalFeature::Wandmaker : JournalFeature::Imp, depth_);
    postMessage(quest == 1 ? "Wandmaker: an excellent choice." : "Imp: a deal is a deal. Take the ring.");
    return true;
}

bool Dungeon::canReforge(const Item& item) const {
    return item.isIdentified() && !item.cursed() && item.level() >= 0 && item.isUpgradable();
}

bool Dungeon::reforge(Item* first, Item* second) {
    if (questState(2) != 650 || !first || !second || first == second || !canReforge(*first) || !canReforge(*second) || !first->sameKind(*second)) return false;
    if (second->level() > first->level()) std::swap(first, second);
    Hero& hero = hero_;
    std::unique_ptr<Item> consumed;
    if (hero.belongings().weapon() == second) consumed = hero.belongings().takeWeapon();
    else if (hero.belongings().armor() == second) consumed = hero.belongings().takeArmor();
    else if (Ring* ring = dynamic_cast<Ring*>(second)) consumed = hero.belongings().takeRing(ring);
    else consumed = hero.belongings().backpack().removeAll(second);
    if (!consumed) return false;
    first->upgrade();
    setQuestState(2, 2);
    Progression::instance().removeJournal(JournalFeature::Troll, depth_);
    postMessage("Blacksmith: your item certainly looks better now.");
    return true;
}
void Dungeon::processQuestKill(const Mob& mob, const void* cause) {
    if (developerTestRoom_) {
        const int kind=testMobKind(mob);
        if(kind>=0){ std::unique_ptr<Actor> timer(new TestRespawnActor(*this,kind,mob.pos)); Actor* raw=timer.get(); developerRespawnActors_.push_back(std::move(timer)); scheduler_.addDelayed(raw,15.0f); }
    }
    const int state = questState(0);
    if (state >= 101 && state <= 108 && mob.hostile()) {
        const int left = state - 100;
        if (Random::Int(left) == 0) {
            level_.drop(std::unique_ptr<Item>(new SpecialItem(SpecialKind::DriedRose)), mob.pos);
            setQuestState(0, 150);
            postMessage("A dried rose falls from the defeated creature.");
        } else {
            setQuestState(0, 100 + left - 1);
        }
    }

    // Ghost.Quest creates the fetid rat only after the request has been
    // accepted and a subsequent hostile creature dies on the same floor.
    if (state == 200) {
        bool ratPresent = false;
        for (const auto& candidate : level_.mobs()) {
            const SpecialMob* special = candidate ? dynamic_cast<const SpecialMob*>(candidate.get()) : nullptr;
            if (special && special->kind() == SpecialMobKind::FetidRat) {
                ratPresent = true;
                break;
            }
        }
        if (!ratPresent) {
            const int cell = level_.randomRespawnCell();
            if (spawnSpecialMob(SpecialMobKind::FetidRat, cell)) {
                setQuestState(0, 201);
                postMessage("The fetid rat emerges from the darkness.");
            }
        }
    }

    if (questState(2) != 602 || cause != &hero_) return;
    const ZoneMob* zone = dynamic_cast<const ZoneMob*>(&mob);
    QuestPickaxe* pick = dynamic_cast<QuestPickaxe*>(hero_.belongings().weapon());
    if (zone && zone->kind() == ZoneMobKind::Bat && pick) {
        pick->stain();
        postMessage("The pickaxe is stained with bat blood.");
    }
}
void Dungeon::completeRun(){won_=true;if(!runFinalized_){Progression::instance().finishRun(true);runFinalized_=true;}save();const std::string& path=StoragePaths::saveGame();
#if defined(PD_PLATFORM_PS1)
PS1MemoryCard::removeFile(path);PS1MemoryCard::removeFile(path+".tmp");PS1MemoryCard::removeFile(path+".bak");
#else
std::remove(path.c_str());std::remove((path+".tmp").c_str());std::remove((path+".bak").c_str());
#endif
}



std::unique_ptr<Item> Dungeon::rollChestReward() {
    // Each Luck rank grants 6% chance for a chest to roll from the equipment
    // pool instead of the normal all-item table.
    if (hero_.luckUpgrades() > 0 && Random::Int(100) < std::min(45, hero_.luckUpgrades() * 6)) {
        static const Generator::Category rare[] = {
            Generator::Category::Weapon, Generator::Category::Armor,
            Generator::Category::Wand, Generator::Category::Ring
        };
        return Generator::random(rare[Random::Int(4)], depth_);
    }
    return Generator::random(depth_);
}

bool Dungeon::openChestAt(int target) {
    if (hero_.paralysed) {
        hero_.spendAndNext(Actor::TICK);
        finishHeroTurn();
        return false;
    }
    if (!hero_.isAlive() || !level_.inBoundsCell(target) || !Level::adjacent(hero_.pos, target)) return false;

    Heap* heap = level_.heapAt(target);
    if (!heap || heap->empty()) return false;
    Item* top = heap->peek();
    if (!top) return false;

    const auto revealChest = [&]() {
        SpecialItem* marker = dynamic_cast<SpecialItem*>(heap->peek());
        if (marker && marker->kind() == SpecialKind::ChestPlaceholder) {
            heap->pickUp();
            std::unique_ptr<Item> reward = rollChestReward();
            if (!reward) reward = std::unique_ptr<Item>(new Food());
            heap->drop(std::move(reward));
        }
        top = heap->peek();
    };

    if (heap->type == Heap::Type::Mimic) {
        if (!spawnMimicAt(target)) return false;
        postMessage("The chest is a mimic!");
    } else if (heap->type == Heap::Type::Chest) {
        revealChest();
        if (!top) return false;
        heap->type = Heap::Type::Heap;
        postMessage("Opened chest: " + top->displayName());
    } else if (heap->type == Heap::Type::LockedChest || heap->type == Heap::Type::CrystalChest) {
        Item* key = hero_.belongings().getKey<GoldenKey>();
        if (!key) {
            postMessage("The chest is locked");
            return false;
        }
        hero_.belongings().detach(key);
        revealChest();
        if (!top) return false;
        heap->type = Heap::Type::Heap;
        postMessage("Unlocked chest: " + top->displayName());
    } else {
        return false;
    }

    hero_.spendAndNext(Actor::TICK);
    finishHeroTurn();
    return true;
}

bool Dungeon::pickUpAtHero() {
    if (hero_.paralysed) {
        hero_.spendAndNext(Actor::TICK);
        finishHeroTurn();
        return false;
    }
    if (!hero_.isAlive()) return false;
    Heap* heap = level_.heapAt(hero_.pos);
    if (!heap || heap->empty()) return false;

    if(heap->type==Heap::Type::Tomb){for(int offset:Level::Neighbours4)spawnSpecialMob(SpecialMobKind::Wraith,hero_.pos+offset,2.0f);heap->type=Heap::Type::Heap;postMessage("Vengeful wraiths rise from the tomb!");hero_.spendAndNext(Actor::TICK);finishHeroTurn();return true;}
    if(heap->type==Heap::Type::Skeleton){bool cursed=false;for(const auto&item:heap->items())if(item&&item->cursed()){cursed=true;break;}if(cursed&&!spawnSpecialMob(SpecialMobKind::Wraith,hero_.pos,2.0f))hero_.damage(hero_.HP/2,heap);heap->type=Heap::Type::Heap;postMessage(cursed?"The remains are cursed!":"The bones rattle.");hero_.spendAndNext(Actor::TICK);finishHeroTurn();return true;}
    if(heap->type==Heap::Type::Hidden){heap->type=Heap::Type::Heap;postMessage("You reveal a hidden cache.");hero_.spendAndNext(Actor::TICK);finishHeroTurn();return true;}

    Item* top = heap->peek();
    if (!top) return false;
    const auto revealChest=[&](){
        SpecialItem* marker=dynamic_cast<SpecialItem*>(heap->peek());
        if(marker&&marker->kind()==SpecialKind::ChestPlaceholder){
            heap->pickUp();
            std::unique_ptr<Item> reward=rollChestReward();
            if(!reward)reward=std::unique_ptr<Item>(new Food());
            heap->drop(std::move(reward));
        }
        top=heap->peek();
    };
    if(heap->type==Heap::Type::Mimic){if(!spawnMimicAt(hero_.pos))return false;postMessage("The chest is a mimic!");hero_.spendAndNext(Actor::TICK);finishHeroTurn();return true;}
    if(heap->type==Heap::Type::Chest){
        revealChest();if(!top)return false;
        heap->type=Heap::Type::Heap;postMessage("Opened chest: "+top->displayName());hero_.spendAndNext(Actor::TICK);finishHeroTurn();return true;
    }
    if(heap->type==Heap::Type::LockedChest||heap->type==Heap::Type::CrystalChest){
        Item*key=hero_.belongings().getKey<GoldenKey>();if(!key){postMessage("The chest is locked");return false;}
        hero_.belongings().detach(key);revealChest();if(!top)return false;heap->type=Heap::Type::Heap;postMessage("Unlocked chest: "+top->displayName());hero_.spendAndNext(Actor::TICK);finishHeroTurn();return true;
    }
    const bool forSale=heap->type==Heap::Type::ForSale;
    bool haggling=false;for(const Ring*ring:{hero_.belongings().ring1(),hero_.belongings().ring2()})if(ring&&ring->haggling()){haggling=true;break;}const int salePrice=haggling&&top->price()>=2?top->price()/2:top->price();
    if(forSale&&gold_<salePrice){postMessage("You do not have enough gold.");return false;}

    SpecialItem*groundSpecial=dynamic_cast<SpecialItem*>(top);
    if(groundSpecial&&groundSpecial->kind()==SpecialKind::Dewdrop){
        // Dew is consumed from the ground immediately.  Keeping it in the vial
        // here made a normal pickup fail to provide its expected 1 HP recovery.
        std::unique_ptr<Item> dew=heap->pickUp();
        if(!dew)return false;
        hero_.HP=std::min(hero_.HT,hero_.HP+1);
        postMessage("Picked up dewdrop (+1 HP)");
    }
    else if (dynamic_cast<Gold*>(top)) {
        const int pickedGold=top->quantity();
        std::unique_ptr<Item> item = heap->pickUp();
        Gold* picked = dynamic_cast<Gold*>(item.get());
        if (!picked) return false;
        addGold(picked->quantity());
        postMessage("Picked up "+numberToString(pickedGold)+" gold");
    } else {
        const std::string pickedName=top->displayName();
        if (!hero_.belongings().backpack().canCollect(*top)) return false;
        if(forSale&&!spendGold(salePrice))return false;
        if(forSale&&haggling)Progression::instance().noteHaggler();
        const int itemLevel=top->level();
        const bool isAmulet=groundSpecial&&groundSpecial->kind()==SpecialKind::Amulet;
        std::unique_ptr<Item> item = heap->pickUp();
        if (!item) return false;
        if (!hero_.belongings().backpack().collect(std::move(item))) return false;
        if(isAmulet){amuletJustPickedUp_=true;Progression::instance().noteAmuletObtained();}
        Progression::instance().noteItem(itemLevel);
        postMessage("Picked up "+pickedName);
    }

    level_.removeEmptyHeap(hero_.pos);
    hero_.spendAndNext(Item::TimeToPickUp);
    finishHeroTurn();
    return true;
}

bool Dungeon::eatFood() {
    return eatFood(
        hero_
            .belongings()
            .getItem<Food>()
    );
}

bool Dungeon::eatFood(Food* food) {
    if (hero_.paralysed) {
        hero_.spendAndNext(Actor::TICK);
        finishHeroTurn();
        return false;
    }

    if (!hero_.isAlive() ||
        !food) {
        return false;
    }

    if (!hero_.eatFood(food)) {
        return false;
    }
    Progression::instance().noteFood();

    finishHeroTurn();
    return true;
}

bool Dungeon::assignPrimaryQuickSlot(
    Item* item
) {
    if (!item) {
        return false;
    }

    if (dynamic_cast<Dart*>(item)) {
        hero_
            .primaryQuickSlot()
            .setClass(
                QuickSlotKind::Dart
            );

        return true;
    }

    if (WandOfMagicMissile* wand =
            dynamic_cast<
                WandOfMagicMissile*
            >(item)) {

        hero_
            .primaryQuickSlot()
            .setItem(
                QuickSlotKind::
                    WandOfMagicMissile,
                wand
            );

        return true;
    }

    if (Wand* wand=dynamic_cast<Wand*>(item)){hero_.primaryQuickSlot().setItem(QuickSlotKind::Wand,wand);return true;}
    if (MissileWeapon* missile=dynamic_cast<MissileWeapon*>(item)){hero_.primaryQuickSlot().setItem(QuickSlotKind::Missile,missile);return true;}

    if (Boomerang* boomerang =
            dynamic_cast<
                Boomerang*
            >(item)) {

        hero_
            .primaryQuickSlot()
            .setItem(
                QuickSlotKind::
                    Boomerang,
                boomerang
            );

        return true;
    }

    // The slot is player-selected, not a wand/missile-only shortcut.  This
    // supports potions, bombs, seeds and any other item the player chooses to
    // throw through the R1 targeting cursor.
    hero_.primaryQuickSlot().setItem(QuickSlotKind::Item, item);
    return true;
}

bool Dungeon::unlockAt(int target) {
    if (hero_.paralysed) {
        hero_.spendAndNext(Actor::TICK);
        finishHeroTurn();
        return false;
    }
    if (!hero_.isAlive() || !level_.inBoundsCell(target) || !Level::adjacent(hero_.pos, target)) return false;
    const int terrain = level_.cell(target);
    Item* key = nullptr;

    if (terrain == Terrain::LOCKED_DOOR) {
        key = hero_.belongings().getKey<IronKey>();
    } else if (terrain == Terrain::LOCKED_EXIT) {
        key = hero_.belongings().getKey<SkeletonKey>();
    } else {
        return false;
    }

    if (!key) return false;
    auto consumed = hero_.belongings().detach(key);
    if (!consumed) return false;

    level_.setRaw(target, terrain == Terrain::LOCKED_DOOR ? Terrain::DOOR : Terrain::UNLOCKED_EXIT);
    level_.buildFlagMaps();
    hero_.spendAndNext(Key::TimeToUnlock);
    finishHeroTurn();
    return true;
}

int Dungeon::search(bool intentional, bool spendTime) {
    if (!hero_.isAlive()) return 0;
    if (spendTime && hero_.paralysed) {
        hero_.spendAndNext(Actor::TICK);
        finishHeroTurn();
        return 0;
    }

    int positive=0,negative=0;
    for(const Ring*ring:{hero_.belongings().ring1(),hero_.belongings().ring2()}) if(ring) {
        const int bonus=ring->detectionBonus();
        if(bonus>positive) positive=bonus;
        else if(bonus<0) negative+=bonus;
    }
    int radius=1+positive+negative;
    float chance=intentional ? 2.0f*hero_.awareness()-hero_.awareness()*hero_.awareness() : hero_.awareness();
    if(radius<=0) { chance/=2-radius; radius=1; }
    int found = 0;
    const int hx = hero_.pos % Level::Width;
    const int hy = hero_.pos / Level::Width;
    for (int dy = -radius; dy <= radius; ++dy) {
        for (int dx = -radius; dx <= radius; ++dx) {
            const int x = hx + dx;
            const int y = hy + dy;
            if (!level_.inBounds(x, y)) continue;
            const int cell = y * Level::Width + x;
            if (!visible_[static_cast<std::size_t>(cell)]) continue;
            const int terrain = level_.cell(cell);
            if ((Terrain::flags()[static_cast<std::size_t>(terrain)] & Terrain::SECRET) != 0 &&
                (intentional || Random::Float()<chance)) {
                level_.setRaw(cell,Terrain::discover(terrain));
                ++found;
            }
            if(intentional) if(Heap*heap=level_.heapAt(cell)) if(heap->type==Heap::Type::Hidden) {
                heap->type=Heap::Type::Heap;
                ++found;
            }
        }
    }
    if (found > 0) level_.buildFlagMaps();
    if (spendTime) {
        hero_.spendAndNext(found>0 && Random::Float()>=chance ? Actor::TICK*2.0f : Actor::TICK);
        finishHeroTurn();
    }
    observe();
    return found;
}

int Dungeon::searchAdjacent() {
    return search(true, true);
}

int Dungeon::firstVisibleMobCell() const {
    for (const auto& mob : level_.mobs()) {
        if (mob && mob->isAlive() && mob->hostile() && visible_[static_cast<std::size_t>(mob->pos)]) return mob->pos;
    }
    return -1;
}

bool Dungeon::usePrimaryQuickSlot(int target) {
    if (hero_.paralysed) {
        hero_.spendAndNext(Actor::TICK);
        finishHeroTurn();
        return false;
    }
    if (!hero_.isAlive()) return false;
    Item* item = hero_.primaryQuickSlot().select(hero_);
    if (!item) return false;
    if (MissileWeapon* missile = dynamic_cast<MissileWeapon*>(item)) return throwMissileAt(missile, target);
    if (Wand* wand = dynamic_cast<Wand*>(item)) return zapWandAt(wand, target);
    return throwItemAt(item, target);
}

bool Dungeon::throwMissileAt(MissileWeapon* weapon, int target) {
    if (!weapon || !hero_.isAlive() || !level_.inBoundsCell(target)) return false;
    const int impact = Ballistica::cast(*this, hero_.pos, target, false, true);
    std::unique_ptr<Item> owned = hero_.belongings().detach(weapon);
    if (!owned) return false;
    MissileWeapon* projectile = dynamic_cast<MissileWeapon*>(owned.get());
    if (!projectile) return false;
    Char* enemy = scheduler_.findChar(impact);
    bool zeroed=false;
    if(enemy){if(SnipersMark*mark=dynamic_cast<SnipersMark*>(hero_.findBuff(typeid(SnipersMark)))){zeroed=mark->object==enemy->id();mark->detach();}}
    bool hit = false;
    if (enemy && enemy != &hero_) hit = hero_.shoot(*enemy, *projectile);
    const float delay = projectile->speedFactor(hero_)*(zeroed?0.5f:1.0f);
    if (projectile->returnsToOwner()) {
        if (hero_.belongings().backpack().canCollect(*owned)) {
            hero_.belongings().backpack().collect(std::move(owned));
        } else {
            level_.drop(std::move(owned), hero_.pos);
        }
    } else {
        // A thrown weapon is recoverable whether it hits or misses.  Special
        // projectiles may still handle their own miss effect first.
        if (!hit) projectile->onMiss(*this, impact);
        if (owned) level_.drop(std::move(owned), impact);
    }
    level_.press(impact, nullptr);
    hero_.spendAndNext(delay);
    finishHeroTurn();
    return true;
}

bool Dungeon::zapWandAt(Wand* wand, int target) {
    if (!wand || !hero_.isAlive() || !level_.inBoundsCell(target)) return false;
    if (!wand->zap(*this, hero_, target)) return false;
    finishHeroTurn();
    level_.purgeDeadMobs();
    observe();
    return true;
}


bool Dungeon::drinkPotion(Potion* potion) {
    if (!potion ||
        !hero_.isAlive() ||
        hero_.paralysed) {
        return false;
    }

    std::unique_ptr<Item> owned =
        hero_
            .belongings()
            .detach(potion);

    Potion* consumed =
        dynamic_cast<Potion*>(
            owned.get()
        );

    if (!consumed) {
        return false;
    }

    consumed->apply(
        *this,
        hero_
    );

    hero_.spendAndNext(
        Potion::TimeToDrink
    );

    finishHeroTurn();
    return true;
}

bool Dungeon::throwPotionAt(
    Potion* potion,
    int target
) {
    if (!potion ||
        !hero_.isAlive() ||
        hero_.paralysed ||
        !level_.inBoundsCell(target)) {
        return false;
    }

    // Potions use the same ballistic collision as every other thrown item.
    // The old path applied their effect to the selected cell directly, so a
    // flask could pass through a wall and shatter behind it.
    const int impact = Ballistica::cast(*this, hero_.pos, target, false, true);

    std::unique_ptr<Item> owned =
        hero_
            .belongings()
            .detach(potion);

    Potion* thrown =
        dynamic_cast<Potion*>(
            owned.get()
        );

    if (!thrown) {
        return false;
    }

    // The original keeps a flask intact when it lands in a well or a chasm.
    // In every other case it shatters (or is drunk when thrown at the hero).
    if (level_.cell(impact) == Terrain::WELL || level_.pit(impact)) {
        level_.drop(std::move(owned), impact);
    } else {
        thrown->onThrow(
            *this,
            hero_,
            impact
        );
    }
    level_.press(impact, nullptr);

    hero_.spendAndNext(
        Item::TimeToThrow
    );

    finishHeroTurn();
    return true;
}

bool Dungeon::equipItem(Item* item) {
    if (!item || !hero_.isAlive()) return false;
    if (dynamic_cast<KindOfWeapon*>(item)) {
        if (KindOfWeapon* old=hero_.belongings().weapon()) {
            if (old->cursed()) { old->setCursedKnown(true); postMessage("You can't remove cursed "+old->displayName()+"!"); return false; }
        }
        std::unique_ptr<Item> raw=hero_.belongings().backpack().removeAll(item);
        if (!raw) return false;
        std::unique_ptr<KindOfWeapon> incoming(dynamic_cast<KindOfWeapon*>(raw.release()));
        std::unique_ptr<KindOfWeapon> old=hero_.belongings().takeWeapon();
        if (old) { if (hero_.belongings().backpack().canCollect(*old)) hero_.belongings().backpack().collect(std::move(old)); else level_.drop(std::move(old),hero_.pos); }
        hero_.belongings().equipWeapon(std::move(incoming));
        hero_.spendAndNext(1.0f); finishHeroTurn(); return true;
    }
    if (dynamic_cast<Armor*>(item)) {
        if(ClassArmor*classArmor=dynamic_cast<ClassArmor*>(item))if(static_cast<int>(classArmor->kind())!=static_cast<int>(hero_.heroClass()))return false;
        if (Armor* old=hero_.belongings().armor()) {
            if (old->cursed()) { old->setCursedKnown(true); postMessage("You can't remove cursed "+old->displayName()+"!"); return false; }
        }
        std::unique_ptr<Item> raw=hero_.belongings().backpack().removeAll(item);
        if (!raw) return false;
        std::unique_ptr<Armor> incoming(dynamic_cast<Armor*>(raw.release()));
        std::unique_ptr<Armor> old=hero_.belongings().takeArmor();
        if (old) { if (hero_.belongings().backpack().canCollect(*old)) hero_.belongings().backpack().collect(std::move(old)); else level_.drop(std::move(old),hero_.pos); }
        hero_.belongings().equipArmor(std::move(incoming));
        hero_.spendAndNext(2.0f/hero_.speed()); finishHeroTurn(); return true;
    }
    if (dynamic_cast<Ring*>(item)) {
        if(hero_.belongings().ring1()&&hero_.belongings().ring2())return false;
        std::unique_ptr<Item> raw=hero_.belongings().backpack().removeAll(item);
        if(!raw)return false;
        std::unique_ptr<Ring> incoming(dynamic_cast<Ring*>(raw.release()));
        const bool detectsSecrets=incoming&&incoming->detectsSecrets();
        if(!incoming||!hero_.belongings().equipRing(std::move(incoming)))return false;
        // RingOfDetection.doEquip() in the original performs one immediate,
        // passive search before the equip action consumes its turn.
        if(detectsSecrets) passiveSearchFromDetection();
        hero_.spendAndNext(1.0f);finishHeroTurn();return true;
    }
    return false;
}

bool Dungeon::unequipItem(Item* item) {
    if (!item) return false;
    if (item->cursed()) { item->setCursedKnown(true); postMessage("You can't remove cursed "+item->displayName()+"!"); return false; }
    const bool wasArmor=hero_.belongings().armor()==item;
    std::unique_ptr<Item> owned;
    if (hero_.belongings().weapon()==item) owned=hero_.belongings().takeWeapon();
    else if (hero_.belongings().armor()==item) owned=hero_.belongings().takeArmor();
    else if (hero_.belongings().ring1()==item||hero_.belongings().ring2()==item) owned=hero_.belongings().takeRing(dynamic_cast<Ring*>(item));
    else return false;
    if (hero_.belongings().backpack().canCollect(*owned)) hero_.belongings().backpack().collect(std::move(owned)); else level_.drop(std::move(owned),hero_.pos);
    hero_.spendAndNext(wasArmor?2.0f/hero_.speed():1.0f); finishHeroTurn(); return true;
}

bool Dungeon::dropItem(Item* item) {
    if (!item || !hero_.isAlive()) return false;
    if (hero_.belongings().weapon()==item || hero_.belongings().armor()==item || hero_.belongings().ring1()==item || hero_.belongings().ring2()==item) {
        if (!unequipItem(item)) return false;
    }
    std::unique_ptr<Item> owned=hero_.belongings().backpack().removeAll(item);
    if (!owned) return false;
    level_.drop(std::move(owned),hero_.pos); hero_.spendAndNext(Item::TimeToDrop); finishHeroTurn(); return true;
}

bool Dungeon::throwItemAt(Item* item,int target) {
    if (!item || !level_.inBoundsCell(target)) return false;
    if (MissileWeapon* missile=dynamic_cast<MissileWeapon*>(item)) return throwMissileAt(missile,target);
    if (Potion* potion=dynamic_cast<Potion*>(item)) return throwPotionAt(potion,target);
    const int impact=Ballistica::cast(*this,hero_.pos,target,false,true);
    if(CatalogSeed* seed=dynamic_cast<CatalogSeed*>(item)){
        std::unique_ptr<Item> consumed=hero_.belongings().detach(item);if(!consumed)return false;
        level_.drop(std::move(consumed),impact);
        level_.press(impact,nullptr);
        hero_.spendAndNext(Item::TimeToThrow);finishHeroTurn();return true;
    }
    if(CatalogMisc* misc=dynamic_cast<CatalogMisc*>(item)){
        std::unique_ptr<Item> consumed=hero_.belongings().detach(item);if(!consumed)return false;
        if(misc->kind()==MiscKind::Bomb){
            // A bomb dropped into a chasm does not detonate on the current
            // floor.  It remains an item just as a regular thrown object does.
            if(level_.pit(impact)){
                level_.drop(std::move(consumed),impact);
            }else{
                bool terrainAffected=false;
                for(int off:Level::Neighbours9){
                    const int p=impact+off;
                    if(!level_.inBoundsCell(p))continue;
                    if(level_.flamable(p)){level_.destroy(p);terrainAffected=true;}
                    if(Char*ch=scheduler_.findChar(p)){
                        const int damage=Random::IntRange(1+depth_,10+depth_*2)-Random::Int(ch->dr());
                        if(damage>0){
                            ch->damage(damage,misc);
                            if(ch->isAlive())Buff::prolong<Paralysis>(*ch,2.0f);
                        }
                    }
                }
                if(terrainAffected){level_.buildFlagMaps();observe();}
            }
        }else{
            // Honeypot follows the same chasm rule as the original: it is
            // dropped intact instead of releasing a bee on this floor.
            if(level_.pit(impact)){
                level_.drop(std::move(consumed),impact);
            }else{
                int beeCell=impact;if(scheduler_.findChar(beeCell)){beeCell=-1;for(int off:Level::Neighbours4){const int p=impact+off;if(level_.inBoundsCell(p)&&level_.passable(p)&&!scheduler_.findChar(p)){beeCell=p;break;}}}if(beeCell>=0)spawnSpecialMob(SpecialMobKind::Bee,beeCell);
            }
        }
        hero_.spendAndNext(Item::TimeToThrow);finishHeroTurn();return true;
    }
    std::unique_ptr<Item> owned=hero_.belongings().detach(item);
    if (!owned) return false;
    level_.drop(std::move(owned),impact); level_.press(impact,nullptr); hero_.spendAndNext(Item::TimeToThrow); finishHeroTurn(); return true;
}

bool Dungeon::executeItemAction(Item* item,ItemAction action,int target) {
    if (action >= ItemAction::Gladiator && action <= ItemAction::Warden) {
        SpecialItem* tome=dynamic_cast<SpecialItem*>(item);
        if(!tome||tome->kind()!=SpecialKind::TomeOfMastery||hero_.hasBuff(typeid(Blindness)))return false;
        HeroSubClass chosen=HeroSubClass::None;
        switch(action){case ItemAction::Gladiator:chosen=HeroSubClass::Gladiator;break;case ItemAction::Berserker:chosen=HeroSubClass::Berserker;break;case ItemAction::Battlemage:chosen=HeroSubClass::Battlemage;break;case ItemAction::Warlock:chosen=HeroSubClass::Warlock;break;case ItemAction::Assassin:chosen=HeroSubClass::Assassin;break;case ItemAction::Freerunner:chosen=HeroSubClass::Freerunner;break;case ItemAction::Sniper:chosen=HeroSubClass::Sniper;break;case ItemAction::Warden:chosen=HeroSubClass::Warden;break;default:return false;}
        const std::vector<ItemAction> allowed=tome->actions(hero_);
        if(std::find(allowed.begin(),allowed.end(),action)==allowed.end())return false;
        auto used=hero_.belongings().detach(tome);if(!used)return false;
        hero_.setSubClass(chosen);
        Progression::instance().noteMastery(hero_.heroClass());
        if(chosen==HeroSubClass::Berserker&&hero_.HP<=hero_.HT*Fury::Level)Buff::affect<Fury>(hero_);
        hero_.spendAndNext(10.0f);finishHeroTurn();return true;
    }
    switch(action){
        case ItemAction::Equip:return equipItem(item); case ItemAction::Unequip:return unequipItem(item);
        case ItemAction::Drop:return dropItem(item); case ItemAction::Throw:return throwItemAt(item,target);
        case ItemAction::Eat:return eatFood(dynamic_cast<Food*>(item));
        case ItemAction::Drink:return drinkPotion(dynamic_cast<Potion*>(item));
        case ItemAction::Read:return readScroll(dynamic_cast<Scroll*>(item));
        case ItemAction::Zap:return zapWandAt(dynamic_cast<Wand*>(item),target);
        case ItemAction::QuickSlot:return assignPrimaryQuickSlot(item);
        case ItemAction::Shatter:{
            CatalogMisc* pot=dynamic_cast<CatalogMisc*>(item);
            if(!pot||pot->kind()!=MiscKind::Honeypot)return false;
            auto used=hero_.belongings().detach(pot);if(!used)return false;
            int beeCell=-1;
            for(int off:Level::Neighbours4){const int p=hero_.pos+off;if(level_.inBoundsCell(p)&&level_.passable(p)&&!scheduler_.findChar(p)){beeCell=p;break;}}
            if(beeCell>=0)spawnSpecialMob(SpecialMobKind::Bee,beeCell);
            hero_.spendAndNext(Item::TimeToThrow);finishHeroTurn();return true;
        }
        case ItemAction::Mine:{QuestPickaxe*pick=dynamic_cast<QuestPickaxe*>(item);if(!pick||depth_<11||depth_>15)return false;int vein=-1;for(int off:Level::Neighbours8){const int cell=hero_.pos+off;if(level_.inBoundsCell(cell)&&level_.cell(cell)==Terrain::WALL_DECO){vein=cell;break;}}if(vein<0){postMessage("There is no dark gold vein nearby.");return false;}level_.setRaw(vein,Terrain::WALL);level_.buildFlagMaps();std::unique_ptr<Item>gold(new SpecialItem(SpecialKind::DarkGold));if(!hero_.belongings().backpack().collect(std::move(gold)))level_.drop(std::unique_ptr<Item>(new SpecialItem(SpecialKind::DarkGold)),hero_.pos);if(hero_.hunger()&&!hero_.hunger()->isStarving())hero_.hunger()->satisfy(-Hunger::Starving/10.0f);hero_.spendAndNext(2.0f);finishHeroTurn();return true;}
        case ItemAction::BalanceSpeed:case ItemAction::BalanceAccuracy:{SpecialItem*stone=dynamic_cast<SpecialItem*>(item);Weapon*weapon=dynamic_cast<Weapon*>(hero_.belongings().weapon());if(!stone||stone->kind()!=SpecialKind::Weightstone||!weapon||weapon->isMissileWeapon())return false;auto used=hero_.belongings().detach(stone);if(!used)return false;weapon->imbue(action==ItemAction::BalanceSpeed?Weapon::Imbue::Speed:Weapon::Imbue::Accuracy);hero_.spendAndNext(2.0f);finishHeroTurn();return true;}
        case ItemAction::HeroicLeap:case ItemAction::MoltenEarth:case ItemAction::SmokeBomb:case ItemAction::SpectralBlades:{
            ClassArmor*armor=dynamic_cast<ClassArmor*>(item);if(!armor||hero_.belongings().armor()!=armor||hero_.HP<3)return false;
            const bool matching=(action==ItemAction::HeroicLeap&&armor->kind()==ClassArmorKind::Warrior)||(action==ItemAction::MoltenEarth&&armor->kind()==ClassArmorKind::Mage)||(action==ItemAction::SmokeBomb&&armor->kind()==ClassArmorKind::Rogue)||(action==ItemAction::SpectralBlades&&armor->kind()==ClassArmorKind::Huntress);if(!matching)return false;
            if(action==ItemAction::HeroicLeap){if(!level_.inBoundsCell(target)||target==hero_.pos)return false;int destination=Ballistica::cast(*this,hero_.pos,target,false,true);if(scheduler_.findChar(destination)&&Ballistica::distance()>1)destination=Ballistica::trace()[static_cast<std::size_t>(Ballistica::distance()-2)];if(!level_.passable(destination)&&!level_.avoid(destination))return false;hero_.HP-=hero_.HP/3;Buff::detach<Invisibility>(hero_);Buff::detach<Shadows>(hero_);if(!relocateChar(hero_,destination))return false;for(int offset:Level::Neighbours8)if(Char*enemy=scheduler_.findChar(hero_.pos+offset))if(enemy!=&hero_)Buff::prolong<Paralysis>(*enemy,3.0f*elementsDurationFactor(*enemy));hero_.spendAndNext(1.0f);}
            else if(action==ItemAction::MoltenEarth){hero_.HP-=hero_.HP/3;for(auto&mob:level_.mobsMutable())if(mob&&mob->isAlive()&&visible_[static_cast<std::size_t>(mob->pos)]){if(Burning*burning=Buff::affect<Burning>(*mob))burning->reignite(*mob);Buff::prolong<Roots>(*mob,3.0f);}hero_.spendAndNext(Actor::TICK);}
            else if(action==ItemAction::SmokeBomb){if(!level_.inBoundsCell(target)||!visible_[static_cast<std::size_t>(target)]||(!level_.passable(target)&&!level_.avoid(target))||scheduler_.findChar(target))return false;hero_.HP-=hero_.HP/3;for(auto&mob:level_.mobsMutable())if(mob&&mob->isAlive()&&visible_[static_cast<std::size_t>(mob->pos)]){Buff::prolong<Blindness>(*mob,2.0f);mob->setState(Mob::State::Wandering);}if(!relocateChar(hero_,target))return false;hero_.spendAndNext(Actor::TICK);}
            else{std::vector<Mob*>targets;for(auto&mob:level_.mobsMutable())if(mob&&mob->isAlive()&&visible_[static_cast<std::size_t>(mob->pos)])targets.push_back(mob.get());if(targets.empty())return false;hero_.HP-=hero_.HP/3;for(Mob*mob:targets)if(mob->isAlive())hero_.attack(*mob);hero_.spendAndNext(hero_.attackDelay());}
            finishHeroTurn();return true;
        }
          case ItemAction::Use:{
              SpecialItem* special=dynamic_cast<SpecialItem*>(item);if(!special)return false;
              switch(special->kind()){
                  case SpecialKind::Dewdrop:{auto used=hero_.belongings().detach(special);if(!used)return false;hero_.HP=std::min(hero_.HT,hero_.HP+1);break;}
                  case SpecialKind::DewVial:{const int volume=special->quantity();if(volume<=0)return false;const double exponent=std::log10(20.0);const int healing=static_cast<int>(std::ceil(std::pow(static_cast<double>(volume),exponent)/20.0*hero_.HT));hero_.HP=std::min(hero_.HT,hero_.HP+healing);special->quantity(0);break;}
                  case SpecialKind::Torch:{auto used=hero_.belongings().detach(special);if(!used)return false;Buff::prolong<Light>(hero_,Light::Duration);break;}
                  case SpecialKind::LloydsBeacon:{if(special->state()<0)special->restoreState(depth_*Level::Length+hero_.pos);else{const int savedDepth=special->state()/Level::Length;const int savedPos=special->state()%Level::Length;if(savedDepth!=depth_||!relocateChar(hero_,savedPos))return false;special->restoreState(-1);}break;}
                  case SpecialKind::ArmorKit:{Armor* old=hero_.belongings().armor();if(!old)return false;auto used=hero_.belongings().detach(special);if(!used)return false;ClassArmorKind kind=static_cast<ClassArmorKind>(static_cast<int>(hero_.heroClass()));std::unique_ptr<ClassArmor> armor(new ClassArmor(kind));armor->level(old->level());if(old->isInscribed())armor->inscribe(static_cast<GlyphKind>(old->glyphId()));armor->identify();hero_.belongings().takeArmor();hero_.belongings().equipArmor(std::move(armor));break;}
                  case SpecialKind::Weightstone:return false;
                  case SpecialKind::Amulet:completeRun();return true;
                  default:return false;
              }
              hero_.spendAndNext(1.0f);finishHeroTurn();return true;
          }
        default:return false;
    } return false;
}



bool Dungeon::canTargetScroll(
    const Scroll& scroll,
    const Item& item
) const {
    switch (scroll.kind()) {
        case ScrollKind::Identify:
            return
                !item.isIdentified();

        case ScrollKind::Upgrade:
            return
                item.isUpgradable();

        case ScrollKind::Enchantment:
            return
                dynamic_cast<const Weapon*>(
                    &item
                ) != nullptr ||
                dynamic_cast<const Armor*>(
                    &item
                ) != nullptr;

        default:
            return false;
    }
}

bool Dungeon::readScroll(
    Scroll* scroll,
    Item* targetItem
) {
    if (!scroll ||
        !hero_.isAlive() ||
        hero_.paralysed) {
        return false;
    }

    if (hero_.hasBuff(
            typeid(Blindness)
        )) {
        // Original Scroll.execute refuses to read while blinded.
        return false;
    }

    const ScrollKind kind =
        scroll->kind();

    if (scroll->requiresInventoryTarget()) {
        if (!targetItem ||
            !canTargetScroll(
                *scroll,
                *targetItem
            )) {
            return false;
        }
    }

    // InventoryScroll identifies itself by use before item selection.
    scroll->setKnown();

    std::unique_ptr<Item> owned =
        hero_
            .belongings()
            .detach(scroll);

    Scroll* consumed =
        dynamic_cast<Scroll*>(
            owned.get()
        );

    if (!consumed) {
        return false;
    }

    bool knownByEffect = true;

    switch (kind) {
        case ScrollKind::Identify: {
            targetItem->identify();
            break;
        }

        case ScrollKind::MagicMapping: {
            auto& mapped =
                level_.mappedMutable();

            const auto& discoverable =
                level_.discoverableMap();

            bool terrainChanged = false;

            for (int i = 0;
                 i < Level::Length;
                 ++i) {

                if (!discoverable[
                        static_cast<std::size_t>(
                            i
                        )
                    ]) {
                    continue;
                }

                mapped[
                    static_cast<std::size_t>(
                        i
                    )
                ] = true;

                const int terrain =
                    level_.cell(i);

                if ((Terrain::flagsFor(
                         terrain
                     ) &
                     Terrain::SECRET) != 0) {

                    level_.setRaw(
                        i,
                        Terrain::discover(
                            terrain
                        )
                    );

                    terrainChanged = true;
                }
            }

            if (terrainChanged) {
                level_.buildFlagMaps();
                level_.cleanWalls();
            }

            observe();
            break;
        }

        case ScrollKind::Teleportation: {
            int count = 10;
            int pos = -1;

            do {
                pos =
                    level_
                        .randomRespawnCell();

                if (count-- <= 0) {
                    break;
                }

            } while (
                pos == -1
            );

            if (pos != -1) relocateChar(hero_,pos);

            break;
        }

        case ScrollKind::Recharging: {
            hero_
                .belongings()
                .charge(true);
            break;
        }

        case ScrollKind::RemoveCurse: {
            hero_
                .belongings()
                .uncurseAll();

            Buff::detach<Weakness>(
                hero_
            );

            break;
        }

        case ScrollKind::Terror: {
            const int source =
                hero_.id();

            for (auto& mob :
                 level_.mobsMutable()) {

                if (!mob ||
                    !mob->isAlive() ||
                    !visible_[
                        static_cast<std::size_t>(
                            mob->pos
                        )
                    ]) {
                    continue;
                }

                Terror* terror =
                    Buff::affect<Terror>(
                        *mob,
                        Terror::Duration
                    );

                if (terror) {
                    terror->object =
                        source;
                }
            }

            break;
        }

        case ScrollKind::Lullaby: {
            for (auto& mob :
                 level_.mobsMutable()) {

                if (!mob ||
                    !mob->isAlive() ||
                    !visible_[
                        static_cast<std::size_t>(
                            mob->pos
                        )
                    ]) {
                    continue;
                }

                Buff::affect<Sleep>(
                    *mob
                );
            }

            break;
        }

        case ScrollKind::Challenge: {
            for (auto& mob :
                 level_.mobsMutable()) {

                if (!mob ||
                    !mob->isAlive()) {
                    continue;
                }

                mob->beckon(
                    hero_.pos
                );

                if (visible_[
                        static_cast<std::size_t>(
                            mob->pos
                        )
                    ]) {

                    Buff::affect<Rage>(
                        *mob,
                        static_cast<float>(
                            Level::distance(
                                hero_.pos,
                                mob->pos
                            )
                        )
                    );
                }
            }

            std::vector<int>mimics;for(const auto&entry:level_.heaps())if(entry.second&&entry.second->type==Heap::Type::Mimic)mimics.push_back(entry.first);for(int pos:mimics)spawnMimicAt(pos);
            break;
        }

        case ScrollKind::PsionicBlast: {
            for (auto& mob :
                 level_.mobsMutable()) {

                if (!mob ||
                    !mob->isAlive() ||
                    !visible_[
                        static_cast<std::size_t>(
                            mob->pos
                        )
                    ]) {
                    continue;
                }

                if (const ZoneBoss* boss = dynamic_cast<const ZoneBoss*>(mob.get())) {
                    if (boss->kind() == ZoneBossKind::Tengu || boss->kind() == ZoneBossKind::DM300 || boss->kind() == ZoneBossKind::King || boss->kind() == ZoneBossKind::Yog) continue;
                }
                if (const ZoneMob* zone = dynamic_cast<const ZoneMob*>(mob.get())) {
                    if (zone->kind() == ZoneMobKind::RottingFist || zone->kind() == ZoneMobKind::BurningFist) continue;
                    if (zone->kind() == ZoneMobKind::Golem) continue;
                    if (zone->kind() == ZoneMobKind::Elemental) continue;
                }
                if (dynamic_cast<const Goo*>(mob.get())) continue;
                if (const SpecialMob* special = dynamic_cast<const SpecialMob*>(mob.get())) {
                    if (special->kind() == SpecialMobKind::Statue) continue;
                }
                if (dynamic_cast<const Mimic*>(mob.get())) continue;

                Buff::prolong<Blindness>(
                    *mob,
                    static_cast<float>(
                        Random::Int(
                            3,
                            6
                        )
                    )
                );

                mob->damage(
                    Random::IntRange(
                        1,
                        std::max(
                            1,
                            mob->HT *
                            2 /
                            3
                        )
                    ),
                    consumed
                );
            }

            Buff::prolong<Blindness>(
                hero_,
                static_cast<float>(
                    Random::Int(
                        3,
                        6
                    )
                )
            );

            observe();
            break;
        }

        case ScrollKind::MirrorImage: {
            std::vector<int> respawn;

            for (int offset :
                 Level::Neighbours8) {

                const int pos =
                    hero_.pos +
                    offset;

                if (!level_
                        .inBoundsCell(pos) ||
                    scheduler_
                        .findChar(pos) !=
                        nullptr ||
                    !(
                        level_.passable(pos) ||
                        level_.avoid(pos)
                     )) {
                    continue;
                }

                respawn.push_back(
                    pos
                );
            }

            int images = 3;
            int created = 0;

            while (
                images > 0 &&
                !respawn.empty()
            ) {
                const int index =
                    Random::Int(
                        static_cast<int>(
                            respawn.size()
                        )
                    );

                std::unique_ptr<MirrorImage> image(
                    new MirrorImage()
                );

                image->duplicate(hero_);
                image->pos =
                    respawn[
                        static_cast<std::size_t>(
                            index
                        )
                    ];

                image->bind(this);

                MirrorImage* raw =
                    image.get();

                level_.addMob(
                    std::move(image)
                );

                scheduler_.add(raw);

                respawn.erase(
                    respawn.begin() +
                    index
                );

                --images;
                ++created;
            }

            knownByEffect =
                created > 0;

            break;
        }

        case ScrollKind::WipeOut:{
            Buff::detach<Invisibility>(hero_);
            // ScrollOfWipeOut iterates every Mob in the original; only bosses
            // survive.  Limiting this to hostile mobs incorrectly spared
            // neutral creatures such as bees and mirror images.
            for(auto& mob:level_.mobsMutable())if(mob&&!mob->isBoss())mob->damage(mob->HP,this);
            for(const auto& entry:level_.heaps())if(entry.second&&(entry.second->type==Heap::Type::ForSale||entry.second->type==Heap::Type::Mimic))entry.second->type=Heap::Type::Heap;
            break;
        }

        case ScrollKind::Upgrade: {
            targetItem->setCursed(false);
            if (targetItem->isBroken()) targetItem->fix();
            else targetItem->upgrade();
            break;
        }

        case ScrollKind::Enchantment: {
            targetItem->setCursed(false);

            if (Weapon* weapon =
                    dynamic_cast<Weapon*>(
                        targetItem
                    )) {

                weapon->enchant();

            } else if (
                Armor* armor =
                    dynamic_cast<Armor*>(
                        targetItem
                    )
            ) {

                armor->inscribe();
            }

            targetItem->fix();

            break;
        }
    }

    if (knownByEffect) {
        consumed->setKnown();
    }

    // Original Invisibility.dispel() only breaks invisibility when there are
    // visible enemies.
    if (firstVisibleMobCell() >= 0) {
        Buff::detach<Invisibility>(
            hero_
        );
    }

    hero_.spendAndNext(
        Scroll::TimeToRead
    );

    finishHeroTurn();

    return true;
}


bool Dungeon::consumeScrollOnCancel(
    Scroll* scroll
) {
    if (!scroll ||
        !hero_.isAlive()) {
        return false;
    }

    std::unique_ptr<Item> owned =
        hero_
            .belongings()
            .detach(scroll);

    if (!owned) {
        return false;
    }

    hero_.spendAndNext(
        Scroll::TimeToRead
    );

    finishHeroTurn();
    return true;
}


bool Dungeon::posNeeded() const {
    static const int quota[] = {4,2,9,4,14,6,19,8,24,9};
    for (int i=0;i<10;i+=2) {
        if (depth_ <= quota[i]) {
            return Random::Float() < static_cast<float>(quota[i+1]-potionOfStrength_) / static_cast<float>(quota[i]-depth_+1);
        }
    }
    return false;
}

bool Dungeon::souNeeded() const {
    static const int quota[] = {5,3,10,6,15,9,20,12,25,13};
    for (int i=0;i<10;i+=2) {
        if (depth_ <= quota[i]) {
            return Random::Float() < static_cast<float>(quota[i+1]-scrollsOfUpgrade_) / static_cast<float>(quota[i]-depth_+1);
        }
    }
    return false;
}

bool Dungeon::soeNeeded() const {
    return Random::Int(12 * (1 + scrollsOfEnchantment_)) < depth_;
}

} // namespace pdv
