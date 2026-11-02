#include <cassert>
#include <cmath>
#include <iostream>
#include <memory>
#include <queue>

#include "core/GameSession.h"
#include "game/Dungeon.h"
#include "game/actors/Hero.h"
#include "game/actors/mobs/Goo.h"
#include "game/actors/mobs/Piranha.h"
#include "game/actors/mobs/ZoneMob.h"
#include "game/actors/mobs/npcs/DungeonNPC.h"
#include "game/items/Bag.h"
#include "game/items/ConsumableCatalog.h"
#include "game/items/EquipmentCatalog.h"
#include "game/items/Generator.h"
#include "game/items/Gold.h"
#include "game/items/Heap.h"
#include "game/items/ItemSpriteSheet.h"
#include "game/items/SpecialItems.h"
#include "game/items/armor/ClothArmor.h"
#include "game/items/bags/Keyring.h"
#include "game/items/bags/ScrollHolder.h"
#include "game/items/bags/SeedPouch.h"
#include "game/items/keys/GoldenKey.h"
#include "game/items/keys/IronKey.h"
#include "game/items/keys/SkeletonKey.h"
#include "game/items/potions/Potion.h"
#include "game/items/potions/PotionOfExperience.h"
#include "game/items/potions/PotionOfMight.h"
#include "game/items/potions/PotionOfStrength.h"
#include "game/items/scrolls/Scroll.h"
#include "game/items/seeds/Seed.h"
#include "game/items/wands/WandOfMagicMissile.h"
#include "game/items/weapon/melee/Dagger.h"
#include "game/items/weapon/melee/Knuckles.h"
#include "game/items/weapon/melee/ShortSword.h"
#include "game/items/weapon/missiles/Boomerang.h"
#include "game/items/weapon/missiles/Dart.h"
#include "game/levels/SewerLevel.h"
#include "game/levels/Terrain.h"
#include "game/utils/Random.h"

using namespace pdv;

class ExposedSewer : public SewerLevel {
public:
  using RegularLevel::nTraps;
};

static bool isTrapTerrain(int t) {
  switch (t) {
  case Terrain::TOXIC_TRAP:
  case Terrain::SECRET_TOXIC_TRAP:
  case Terrain::FIRE_TRAP:
  case Terrain::SECRET_FIRE_TRAP:
  case Terrain::PARALYTIC_TRAP:
  case Terrain::SECRET_PARALYTIC_TRAP:
  case Terrain::POISON_TRAP:
  case Terrain::SECRET_POISON_TRAP:
  case Terrain::ALARM_TRAP:
  case Terrain::SECRET_ALARM_TRAP:
  case Terrain::LIGHTNING_TRAP:
  case Terrain::SECRET_LIGHTNING_TRAP:
  case Terrain::GRIPPING_TRAP:
  case Terrain::SECRET_GRIPPING_TRAP:
  case Terrain::SUMMONING_TRAP:
  case Terrain::SECRET_SUMMONING_TRAP:
    return true;
  default:
    return false;
  }
}

static bool entranceReachesExit(const Level &level) {
  std::array<bool, Level::Length> reached{};
  std::queue<int> pending;
  pending.push(level.entrance());
  reached[static_cast<std::size_t>(level.entrance())] = true;
  while (!pending.empty()) {
    const int cell = pending.front();
    pending.pop();
    if (cell == level.exit())
      return true;
    const int x = cell % Level::Width;
    const int y = cell / Level::Width;
    for (int offset : Level::Neighbours4) {
      const int next = cell + offset;
      if (!level.inBoundsCell(next) || reached[static_cast<std::size_t>(next)])
        continue;
      if (std::abs(next % Level::Width - x) +
              std::abs(next / Level::Width - y) !=
          1)
        continue;
      if (!(level.passable(next) || level.avoid(next)))
        continue;
      reached[static_cast<std::size_t>(next)] = true;
      pending.push(next);
    }
  }
  return false;
}

static void generatorTables() {
  using C = Generator::Category;
  Generator::reset();

  assert(Generator::baseProbability(C::Weapon) == 15);
  assert(Generator::baseProbability(C::Armor) == 10);
  assert(Generator::baseProbability(C::Potion) == 50);
  assert(Generator::baseProbability(C::Scroll) == 40);
  assert(Generator::baseProbability(C::Wand) == 4);
  assert(Generator::baseProbability(C::Ring) == 2);
  assert(Generator::baseProbability(C::Seed) == 5);
  assert(Generator::baseProbability(C::Food) == 0);
  assert(Generator::baseProbability(C::Gold) == 50);
  assert(Generator::baseProbability(C::Misc) == 5);

  const std::array<float, 12> expectedPotions = {
      {45, 4, 15, 10, 15, 10, 0, 20, 12, 10, 0, 10}};
  const std::array<float, 12> expectedScrolls = {
      {30, 10, 15, 10, 15, 12, 8, 8, 4, 6, 0, 1}};
  assert(Generator::potionProbabilities() == expectedPotions);
  assert(Generator::scrollProbabilities() == expectedScrolls);

  assert(Generator::categoryImplemented(C::Potion));
  assert(Generator::categoryImplemented(C::Scroll));
  assert(Generator::categoryImplemented(C::Gold));
  assert(Generator::categoryImplemented(C::Weapon));
  assert(Generator::categoryImplemented(C::Armor));
  assert(Generator::categoryImplemented(C::Wand));
  assert(Generator::categoryImplemented(C::Ring));
  assert(Generator::categoryImplemented(C::Seed));
  assert(Generator::categoryImplemented(C::Misc));

  std::unique_ptr<Item> generatedWeapon = Generator::random(C::Weapon, 1);
  assert(generatedWeapon);
  assert(std::fabs(Generator::currentProbability(C::Weapon) - 7.5f) < 0.0001f);

  assert(Generator::random(C::Armor, 1));
  assert(Generator::random(C::Wand, 1));
  assert(Generator::random(C::Ring, 1));
  assert(Generator::random(C::Seed, 1));
  assert(Generator::random(C::Misc, 1));

  Generator::reset();
  std::unique_ptr<Item> goldItem = Generator::random(C::Gold, 1);
  Gold *gold = dynamic_cast<Gold *>(goldItem.get());
  assert(gold);
  assert(gold->quantity() >= 30 && gold->quantity() < 60);
  assert(gold->image() == ItemSpriteSheet::GOLD);
  assert(std::fabs(Generator::currentProbability(C::Gold) - 25.0f) < 0.0001f);
}

static void completeCatalogGeneration() {
  using C = Generator::Category;
  Generator::reset();
  Random::seed(0xCA7A109u);
  const C categories[] = {C::Weapon, C::Armor, C::Wand,
                          C::Ring,   C::Seed,  C::Misc};
  for (C category : categories)
    for (int i = 0; i < 250; ++i) {
      std::unique_ptr<Item> item = Generator::random(category, 10);
      assert(item);
      assert(item->image() >= 0 && item->image() < 127);
      std::unique_ptr<Item> copy = item->clone();
      assert(copy && copy->name() == item->name());
    }
  for (int i = 0; i < 8; ++i) {
    CatalogSeed seed(static_cast<SeedKind>(i));
    assert(seed.image() == ItemSpriteSheet::SEED_FIREBLOOM + i);
  }
  for (int i = 0; i < 12; ++i) {
    std::unique_ptr<Item> wand =
        i == 10 ? std::unique_ptr<Item>(new WandOfMagicMissile())
                : std::unique_ptr<Item>(new CatalogWand(
                      static_cast<WandKind>(i < 10 ? i : i - 1)));
    assert(wand && dynamic_cast<Wand *>(wand.get()));
    assert(wand->image() != ItemSpriteSheet::SMTH);
  }
  Ring::initGems();
  const int knownRingImages[] = {32, 33, 34, 35, 36, 37, 38, 39, 72, 73, 74};
  for (int i = 0; i < 11; ++i) {
    CatalogRing ring(static_cast<RingKind>(i));
    const int image = ring.image();
    assert(image == ItemSpriteSheet::RING ||
           image == ItemSpriteSheet::RING_EMERALD ||
           image == ItemSpriteSheet::RING_SAPPHIRE ||
           image == ItemSpriteSheet::RING_QUARTZ ||
           image == ItemSpriteSheet::RING_AGATE);
    ring.identify();
    assert(ring.image() == knownRingImages[i]);
  }
  for (int i = 0; i < 4; ++i) {
    ClassArmor armor(static_cast<ClassArmorKind>(i));
    assert(armor.tier() == 6);
  }
  ShortSword shortSword;
  Knuckles knuckles;
  Dagger dagger;
  Dart dart;
  Boomerang boomerang;
  ClothArmor cloth;
  WandOfMagicMissile missile;
  SkeletonKey skeleton;
  IronKey iron;
  GoldenKey golden;
  Keyring keyring;
  const Item *fixed[] = {&shortSword, &knuckles, &dagger,  &dart,
                         &boomerang,  &cloth,    &missile, &skeleton,
                         &iron,       &golden,   &keyring};
  for (const Item *item : fixed)
    assert(item->image() != ItemSpriteSheet::SMTH &&
           item->image() != ItemSpriteSheet::BONES);
}

static void potionAndScrollGeneration() {
  Generator::reset();
  Random::seed(0x150001u);

  for (int i = 0; i < 1000; ++i) {
    std::unique_ptr<Item> item =
        Generator::random(Generator::Category::Potion, 1);
    assert(item);
    Potion *p = dynamic_cast<Potion *>(item.get());
    assert(p);
    // Strength and Might have zero ordinary Generator probability.
    assert(dynamic_cast<PotionOfStrength *>(p) == nullptr ||
           dynamic_cast<PotionOfMight *>(p) != nullptr);
    assert(dynamic_cast<PotionOfMight *>(p) == nullptr);
  }

  Generator::reset();
  Random::seed(0x150002u);
  for (int i = 0; i < 2000; ++i) {
    std::unique_ptr<Item> item =
        Generator::random(Generator::Category::Scroll, 1);
    Scroll *scroll = dynamic_cast<Scroll *>(item.get());
    assert(scroll);
    // ScrollOfUpgrade has zero ordinary Generator weight; it is quota-spawned.
    assert(scroll->kind() != ScrollKind::Upgrade);
  }
}

static void bagFoundations() {
  Bag backpack(19, "backpack");

  std::unique_ptr<Scroll> scroll(new Scroll(ScrollKind::Identify));
  assert(backpack.collect(std::move(scroll)));

  std::unique_ptr<ScrollHolder> holder(new ScrollHolder());
  ScrollHolder *holderPtr = holder.get();
  assert(holderPtr->size() == 12);
  assert(backpack.collect(std::move(holder)));
  assert(holderPtr->count() == 1);
  assert(holderPtr->findFirst<Scroll>() != nullptr);

  std::unique_ptr<Seed> seed(new Seed());
  assert(backpack.collect(std::move(seed)));

  std::unique_ptr<SeedPouch> pouch(new SeedPouch());
  SeedPouch *pouchPtr = pouch.get();
  assert(pouchPtr->size() == 8);
  assert(backpack.collect(std::move(pouch)));
  assert(pouchPtr->count() == 1);
  assert(pouchPtr->findFirst<Seed>() != nullptr);
}

static void potionEffects() {
  Dungeon dummy;
  Hero hero;
  hero.initialize(HeroClass::Mage, 100);

  const int str0 = hero.STR();
  PotionOfStrength strength;
  strength.apply(dummy, hero);
  assert(hero.STR() == str0 + 1);

  hero.HP = hero.HT - 5;
  const int ht0 = hero.HT;
  const int str1 = hero.STR();
  PotionOfMight might;
  might.apply(dummy, hero);
  assert(hero.STR() == str1 + 1);
  assert(hero.HT == ht0 + 5);
  assert(hero.HP == ht0);

  Hero xpHero;
  xpHero.initialize(HeroClass::Mage, 100);
  PotionOfExperience exp;
  exp.apply(dummy, xpHero);
  assert(xpHero.level() == 2);
  assert(xpHero.exp() == 0);
}

static void levelItemAndTrapGeneration() {
  Generator::reset();
  Random::seed(0x150003u);

  ExposedSewer depth1;
  assert(depth1.create(1, 0x150003u));
  int depth1Traps = 0;
  for (int cell : depth1.map())
    if (isTrapTerrain(cell))
      ++depth1Traps;
  assert(depth1Traps == 0);
  // Every non-boss floor adds a FOOD item to itemsToSpawn before random items.
  assert(!depth1.heaps().empty());

  bool foundGeneratedTrap = false;
  for (uint32_t seed = 1; seed < 100 && !foundGeneratedTrap; ++seed) {
    Generator::reset();
    ExposedSewer level;
    assert(level.create(2, 0x150100u + seed));
    const int n = level.nTraps();
    assert(n >= 1);
    assert(n < static_cast<int>(level.rooms().size()) + 2);
    for (int cell : level.map()) {
      if (isTrapTerrain(cell)) {
        foundGeneratedTrap = true;
        break;
      }
    }
  }
  assert(foundGeneratedTrap);

  bool sawGarden = false, sawLibrary = false, sawLaboratory = false,
       sawPool = false;
  for (int depth = 1; depth <= 4; ++depth) {
    for (uint32_t seed = 1; seed <= 100; ++seed) {
      Generator::reset();
      ExposedSewer level;
      assert(level.create(depth, 0xA110000u + seed + depth * 1000u));
      assert(level.cell(level.entrance()) == Terrain::ENTRANCE);
      assert(level.cell(level.exit()) == Terrain::EXIT);
      assert(entranceReachesExit(level));
      int gardens = 0, libraries = 0, laboratories = 0;
      for (const auto &room : level.rooms()) {
        if (room->type == Room::Type::Garden) {
          ++gardens;
          sawGarden = true;
        }
        if (room->type == Room::Type::Library) {
          ++libraries;
          sawLibrary = true;
        }
        if (room->type == Room::Type::Laboratory) {
          ++laboratories;
          sawLaboratory = true;
        }
        if (room->type == Room::Type::Pool) {
          sawPool = true;
          bool hasPiranha = false;
          for (const auto &mob : level.mobs()) {
            if (dynamic_cast<const Piranha *>(mob.get()) &&
                static_cast<const Level &>(level).water(mob->pos))
              hasPiranha = true;
          }
          assert(hasPiranha);
        }
      }
      assert(gardens <= 1 && libraries <= 1 && laboratories <= 1);
    }
  }
  assert(sawGarden && sawLibrary && sawLaboratory && sawPool);

  Generator::reset();
  ExposedSewer bossFloor;
  assert(bossFloor.create(5, 0xA115000u));
  assert(bossFloor.cell(bossFloor.exit()) == Terrain::LOCKED_EXIT);
  const Goo *goo = nullptr;
  int hostileBosses = 0;
  for (const auto &mob : bossFloor.mobs()) {
    if (mob->hostile()) {
      ++hostileBosses;
      goo = dynamic_cast<const Goo *>(mob.get());
    }
  }
  assert(hostileBosses == 1);
  assert(goo);
  assert(goo->HP == 80 && goo->HT == 80);
  assert(goo->expValue() == 10);
  // Depth 5 uses a hand-built arena rather than generated Room metadata.
  // Goo must spawn in the accessible upper combat area, never on the exit.
  assert(static_cast<const Level &>(bossFloor).passable(goo->pos));
  assert(goo->pos / Level::Width < 18);
  assert(goo->pos != bossFloor.exit());
  const int bossEntrance = bossFloor.entrance();
  bossFloor.sealBossEntrance();
  assert(bossFloor.bossEntranceSealed());
  assert(bossFloor.entrance() == 0);
  assert(static_cast<const Level &>(bossFloor).water(bossEntrance));
  bossFloor.unsealBossEntrance();
  assert(!bossFloor.bossEntranceSealed());
  assert(bossFloor.entrance() == bossEntrance);
  assert(bossFloor.cell(bossEntrance) == Terrain::ENTRANCE);
}

static void allZoneGeneration() {
  for (int depth = 6; depth <= 25; ++depth) {
    for (uint32_t seed = 1; seed <= 12; ++seed) {
      Generator::reset();
      ExposedSewer level;
      assert(level.create(depth, 0xD000000u + depth * 1000u + seed));
      assert(level.cell(level.entrance()) == Terrain::ENTRANCE);
      if (depth == 21) {
        assert(entranceReachesExit(level));
        assert(level.mobs().empty());
        assert(level.heaps().empty());
        continue;
      }
      if (depth % 5 == 0) {
        assert(level.cell(level.exit()) == Terrain::LOCKED_EXIT);
      } else {
        assert(entranceReachesExit(level));
        for (const auto &mob : level.mobs())
          assert(mob && level.inBoundsCell(mob->pos));
      }
      if (depth == 6 || depth == 11 || depth == 16) {
        int shops = 0;
        for (const auto &heap : level.heaps())
          if (heap.second->type == Heap::Type::ForSale)
            ++shops;
        assert(shops >= 1);
      }
    } // Quest NPC placement requires a Dungeon context and is covered by
      // integration tests.
  }
}

static void goldPickup() {
  Dungeon dungeon;
  assert(dungeon.startNew(HeroClass::Warrior, 0x150500u));
  const int before = dungeon.gold();
  dungeon.level().drop(std::unique_ptr<Item>(new Gold(37)), dungeon.hero().pos);
  assert(dungeon.pickUpAtHero());
  assert(dungeon.gold() == before + 37);
  assert(dungeon.hero().belongings().getItem<Gold>() == nullptr);
}

static void firstDescentRegression() {
  // Exercise the complete transition path, not merely Level::create().
  // Quest placement and scheduler setup are only present when a Level belongs
  // to Dungeon, which is exactly the state used during the first descent.
  for (uint32_t seed = 1; seed <= 512; ++seed) {
    Dungeon dungeon;
    assert(dungeon.startNew(HeroClass::Warrior, seed));
    dungeon.hero().pos = dungeon.level().exit();
    assert(dungeon.descend());
    assert(dungeon.depth() == 2);
    assert(dungeon.level().cell(dungeon.hero().pos) == Terrain::ENTRANCE);
  }
}

static void npcQuestAndShop() {
  Dungeon dungeon;
  assert(dungeon.startNew(HeroClass::Warrior, 0xA11CEu));
  dungeon.setQuestState(0, 0);
  DungeonNPC ghost(NPCKind::Ghost);
  ghost.bind(&dungeon);
  ghost.interact(dungeon, dungeon.hero());
  // Ghost may choose the curse branch; without a full scheduled level it can
  // legitimately roll back to state 0 when the special mob has no cell.
  assert(dungeon.questState(0) >= 0);
  dungeon.addGold(100);
  const int before = dungeon.gold();
  Heap *sale = dungeon.level().drop(std::unique_ptr<Item>(new Food()),
                                    dungeon.hero().pos);
  assert(sale);
  sale->type = Heap::Type::ForSale;
  assert(dungeon.pickUpAtHero());
  assert(dungeon.gold() < before);
}

int main() {
  generatorTables();
  completeCatalogGeneration();
  potionAndScrollGeneration();
  bagFoundations();
  potionEffects();
  levelItemAndTrapGeneration();
  allZoneGeneration();
  goldPickup();
  firstDescentRegression();
  npcQuestAndShop();

  std::cout << "PIXEL DUNGEON CONSOLE EDITION v0.15 generator + floor items tests: OK\n";
  return 0;
}
