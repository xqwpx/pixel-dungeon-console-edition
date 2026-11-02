#include <cassert>
#include <cmath>
#include <iostream>
#include <memory>

#include "core/GameSession.h"
#include "game/Dungeon.h"
#include "game/actors/Hero.h"
#include "game/actors/mobs/Rat.h"
#include "game/items/Bag.h"
#include "game/items/Heap.h"
#include "game/items/Item.h"
#include "game/items/armor/ClothArmor.h"
#include "game/items/weapon/melee/Dagger.h"
#include "game/items/weapon/melee/Knuckles.h"
#include "game/items/weapon/melee/ShortSword.h"
#include "game/levels/Level.h"
#include "game/utils/Random.h"

using namespace pdv;

static Rat* firstRat(Dungeon& dungeon) {
    for (auto& mob : dungeon.level().mobsMutable()) {
        if (Rat* rat = dynamic_cast<Rat*>(mob.get())) return rat;
    }
    return nullptr;
}

class StackItem : public Item {
public:
    explicit StackItem(int q = 1) {
        name_ = "stack";
        stackable_ = true;
        quantity_ = q;
    }

    std::unique_ptr<Item> clone() const override {
        return std::unique_ptr<Item>(new StackItem(*this));
    }
};

static void warriorLoadout() {
    Hero hero;
    hero.initialize(HeroClass::Warrior, 100);
    Rat rat;

    auto* sword = dynamic_cast<ShortSword*>(hero.belongings().weapon());
    auto* cloth = dynamic_cast<ClothArmor*>(hero.belongings().armor());

    assert(sword && cloth);
    assert(hero.STR() == 11);
    assert(sword->STR() == 11);
    assert(sword->min() == 1 && sword->max() == 12);
    assert(std::fabs(hero.attackDelay() - 1.0f) < 0.0001f);
    assert(hero.attackSkill(rat) == 10);
    assert(hero.dr() == 2);

    for (int i = 0; i < 200; ++i) {
        int damage = hero.damageRoll();
        assert(damage >= 1 && damage <= 12);
    }
}

static void mageLoadout() {
    Hero hero;
    hero.initialize(HeroClass::Mage, 100);
    Rat rat;

    auto* weapon = dynamic_cast<Knuckles*>(hero.belongings().weapon());
    assert(weapon);
    assert(weapon->STR() == 10);
    assert(weapon->min() == 1 && weapon->max() == 5);
    assert(std::fabs(hero.attackDelay() - 0.5f) < 0.0001f);
    assert(hero.attackSkill(rat) == 10);
    assert(hero.defenseSkill(rat) == 5);
    assert(hero.dr() == 2);
}

static void rogueAndHuntressLoadout() {
    Rat rat;

    Hero rogue;
    rogue.initialize(HeroClass::Rogue, 100);
    assert(dynamic_cast<Dagger*>(rogue.belongings().weapon()) != nullptr);
    assert(rogue.attackSkill(rat) == 12);
    assert(rogue.defenseSkill(rat) == 6);
    assert(rogue.dr() == 2);

    Hero huntress;
    huntress.initialize(HeroClass::Huntress, 100);
    assert(dynamic_cast<Dagger*>(huntress.belongings().weapon()) != nullptr);
    assert(huntress.HP == 15 && huntress.HT == 15);
    assert(huntress.attackSkill(rat) == 12);

    for (int i = 0; i < 100; ++i) {
        int damage = huntress.damageRoll();
        assert(damage >= 1 && damage <= 8);
    }
}

static void experienceLevelUp() {
    Hero hero;
    hero.initialize(HeroClass::Mage, 100);
    Rat rat;

    assert(hero.level() == 1 && hero.exp() == 0);
    assert(hero.maxExp() == 10);
    assert(hero.HT == 20);

    hero.earnExp(9);
    assert(hero.level() == 1 && hero.exp() == 9);

    hero.earnExp(1);
    assert(hero.level() == 2 && hero.exp() == 0);
    assert(hero.maxExp() == 15);
    assert(hero.HT == 25 && hero.HP == 25);
    assert(hero.attackSkill(rat) == 11);
    assert(hero.defenseSkill(rat) == 6);

    hero.earnExp(30);
    assert(hero.level() == 3);
    assert(hero.exp() == 15);
    assert(hero.HT == 30);
}

static void ratAwardsExp() {
    Dungeon dungeon;
    assert(dungeon.startNew(HeroClass::Warrior, 0x1984u));
    assert(!dungeon.level().mobs().empty());

    Rat* rat = firstRat(dungeon);
    assert(rat);

    int before = dungeon.hero().exp();
    rat->damage(999, &dungeon.hero());
    assert(dungeon.hero().exp() == before + 1);

    dungeon.level().purgeDeadMobs();
}

static void ratStopsAwardingAfterMaxLevel() {
    Dungeon dungeon;
    assert(dungeon.startNew(HeroClass::Warrior, 0x9191u));

    while (dungeon.hero().level() <= 5) {
        dungeon.hero().earnExp(dungeon.hero().maxExp());
    }

    Rat* rat = firstRat(dungeon);
    assert(rat);

    int before = dungeon.hero().exp();
    rat->damage(999, &dungeon.hero());
    assert(dungeon.hero().exp() == before);

    dungeon.level().purgeDeadMobs();
}

static void bagStacking() {
    Bag bag(19);
    assert(bag.collect(std::unique_ptr<Item>(new StackItem(3))));
    assert(bag.collect(std::unique_ptr<Item>(new StackItem(5))));
    assert(bag.count() == 1);
    assert(bag.items().front()->quantity() == 8);
}

static void heapAndLevelDrop() {
    Heap heap(50);
    heap.drop(std::unique_ptr<Item>(new StackItem(2)));
    heap.drop(std::unique_ptr<Item>(new StackItem(4)));
    assert(heap.size() == 1);
    assert(heap.peek()->quantity() == 6);

    auto picked = heap.pickUp();
    assert(picked && picked->quantity() == 6);
    assert(heap.empty());

    Level level;
    Heap* dropped = level.drop(std::unique_ptr<Item>(new StackItem(1)), 100);
    assert(dropped);
    assert(level.heapAt(100) == dropped);
    assert(dropped->size() == 1);
}

int main() {
    Random::seed(12345);
    warriorLoadout();
    mageLoadout();
    rogueAndHuntressLoadout();
    experienceLevelUp();
    ratAwardsExp();
    ratStopsAwardingAfterMaxLevel();
    bagStacking();
    heapAndLevelDrop();

    std::cout << "PIXEL DUNGEON CONSOLE EDITION v0.9 item + XP parity tests: OK\n";
    return 0;
}
