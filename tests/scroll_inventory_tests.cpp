#include <cassert>
#include <iostream>
#include <memory>
#include <set>
#include <string>

#include "core/GameSession.h"
#include "game/Dungeon.h"
#include "game/actors/Hero.h"
#include "game/actors/buffs/Blindness.h"
#include "game/actors/buffs/Rage.h"
#include "game/actors/buffs/Sleep.h"
#include "game/actors/buffs/Terror.h"
#include "game/actors/mobs/Mob.h"
#include "game/actors/mobs/npcs/MirrorImage.h"
#include "game/items/Item.h"
#include "game/items/armor/Armor.h"
#include "game/items/potions/Potion.h"
#include "game/items/potions/PotionOfHealing.h"
#include "game/items/potions/PotionOfInvisibility.h"
#include "game/items/scrolls/Scroll.h"
#include "game/items/wands/WandOfMagicMissile.h"
#include "game/items/weapon/Weapon.h"
#include "game/levels/Terrain.h"
#include "game/ui/InventoryModel.h"
#include "game/utils/Random.h"

using namespace pdv;

static Scroll* addScroll(
    Hero& hero,
    ScrollKind kind
) {
    std::unique_ptr<Scroll> scroll(
        kind == ScrollKind::Upgrade
        ? static_cast<Scroll*>(
            new ScrollOfUpgrade()
          )
        : kind == ScrollKind::Enchantment
            ? static_cast<Scroll*>(
                new ScrollOfEnchantment()
              )
            : new Scroll(kind)
    );

    Scroll* raw = scroll.get();

    assert(
        hero.belongings()
            .backpack()
            .collect(
                std::move(scroll)
            )
    );

    return raw;
}

static int adjacentPassable(
    const Level& level,
    int center
) {
    for (int off :
         Level::Neighbours8) {

        const int pos =
            center + off;

        if (level.inBoundsCell(pos) &&
            level.passable(pos)) {
            return pos;
        }
    }

    return -1;
}

static void statusHandlerPermutation() {
    Random::seed(0x160001u);

    Potion::initColors();
    Scroll::initLabels();

    std::set<int> potionImages;
    std::set<std::string> potionColors;

    PotionOfHealing heal;
    PotionOfInvisibility invis;

    potionImages.insert(heal.image());
    potionImages.insert(invis.image());

    potionColors.insert(heal.color());
    potionColors.insert(invis.color());

    assert(potionImages.size() == 2);
    assert(potionColors.size() == 2);

    assert(!heal.isKnown());
    heal.setKnown();

    PotionOfHealing secondHeal;
    assert(secondHeal.isKnown());

    Scroll identify(
        ScrollKind::Identify
    );

    Scroll mapping(
        ScrollKind::MagicMapping
    );

    assert(
        identify.rune() !=
        mapping.rune()
    );

    assert(
        identify.image() !=
        mapping.image()
    );

    assert(!identify.isKnown());
    identify.setKnown();

    Scroll identifyAgain(
        ScrollKind::Identify
    );

    assert(identifyAgain.isKnown());
}

static void inventoryLayoutAndFilters() {
    Hero hero;
    hero.initialize(
        HeroClass::Warrior,
        100
    );

    InventoryModel model;
    model.bind(&hero);
    model.open(
        InventoryMode::All,
        "Backpack"
    );

    assert(model.openState());
    assert(
        InventoryModel::Columns == 6
    );

    assert(
        InventoryModel::SlotSize == 25
    );

    assert(
        InventoryModel::SlotMargin == 1
    );

    // Bags are intentionally flattened into the single Vita inventory view.
    assert(model.tabCount() == 1);

    Scroll* upgrade =
        addScroll(
            hero,
            ScrollKind::Upgrade
        );

    model.rebuild();

    model.setMode(
        InventoryMode::Upgradable,
        "Upgrade"
    );

    bool sawWeapon = false;
    bool sawScrollEnabled = false;

    for (const auto& slot :
         model.slots()) {

        if (slot.item ==
            hero.belongings().weapon()) {
            sawWeapon = slot.enabled;
        }

        if (slot.item == upgrade) {
            sawScrollEnabled =
                slot.enabled;
        }
    }

    assert(sawWeapon);
    assert(!sawScrollEnabled);
}

static void identifyAndUpgradeScrolls() {
    Dungeon dungeon;

    assert(
        dungeon.startNew(
            HeroClass::Warrior,
            0x160010u
        )
    );

    Hero& hero =
        dungeon.hero();

    std::unique_ptr<PotionOfHealing> potion(
        new PotionOfHealing()
    );

    PotionOfHealing* potionRaw =
        potion.get();

    assert(
        hero.belongings()
            .backpack()
            .collect(
                std::move(potion)
            )
    );

    assert(!potionRaw->isKnown());

    Scroll* identify =
        addScroll(
            hero,
            ScrollKind::Identify
        );

    assert(
        dungeon.canTargetScroll(
            *identify,
            *potionRaw
        )
    );

    assert(
        dungeon.readScroll(
            identify,
            potionRaw
        )
    );

    assert(potionRaw->isKnown());

    Weapon* weapon =
        dynamic_cast<Weapon*>(
            hero.belongings()
                .weapon()
        );

    assert(weapon);

    const int oldLevel =
        weapon->level();

    Scroll* upgrade =
        addScroll(
            hero,
            ScrollKind::Upgrade
        );

    assert(
        dungeon.readScroll(
            upgrade,
            weapon
        )
    );

    assert(
        weapon->level() ==
        oldLevel + 1
    );

    Scroll* enchant =
        addScroll(
            hero,
            ScrollKind::Enchantment
        );

    assert(
        dungeon.readScroll(
            enchant,
            weapon
        )
    );

    assert(
        weapon->isEnchanted()
    );
}

static void mappingAndRechargeScrolls() {
    Dungeon dungeon;

    assert(
        dungeon.startNew(
            HeroClass::Mage,
            0x160020u
        )
    );

    Hero& hero =
        dungeon.hero();

    const int secret =
        adjacentPassable(
            dungeon.level(),
            hero.pos
        );

    assert(secret >= 0);

    dungeon.level()
        .setRaw(
            secret,
            Terrain::SECRET_DOOR
        );

    dungeon.level()
        .buildFlagMaps();

    Scroll* mapping =
        addScroll(
            hero,
            ScrollKind::MagicMapping
        );

    assert(
        dungeon.readScroll(
            mapping
        )
    );

    assert(
        dungeon.level()
            .cell(secret) ==
        Terrain::DOOR
    );

    assert(
        dungeon.level()
            .mapped()[
                static_cast<std::size_t>(
                    secret
                )
            ]
    );

    WandOfMagicMissile* wand =
        hero.belongings()
            .getItem<
                WandOfMagicMissile
            >();

    assert(wand);

    Mob* chargeTarget =
        dungeon.level()
            .mobsMutable()
            .front()
            .get();

    assert(chargeTarget);

    const int target =
        adjacentPassable(
            dungeon.level(),
            hero.pos
        );

    assert(target >= 0);

    dungeon.scheduler()
        .freeCell(chargeTarget->pos);

    chargeTarget->pos = target;

    dungeon.scheduler()
        .occupyCell(chargeTarget);

    dungeon.observe();

    const int chargesBefore =
        wand->curCharges();

    assert(
        dungeon.zapWandAt(
            wand,
            target
        )
    );

    assert(
        wand->curCharges() <
        chargesBefore
    );

    if (wand->curCharges() <
        wand->maxCharges()) {

        Scroll* recharge =
            addScroll(
                hero,
                ScrollKind::Recharging
            );

        assert(
            dungeon.readScroll(
                recharge
            )
        );

        assert(
            wand->curCharges() ==
            wand->maxCharges()
        );
    }
}

static void terrorLullabyChallengePsionic() {
    Dungeon dungeon;

    assert(
        dungeon.startNew(
            HeroClass::Warrior,
            0x160030u
        )
    );

    Hero& hero =
        dungeon.hero();

    assert(
        !dungeon.level()
            .mobsMutable()
            .empty()
    );

    Mob* mob =
        dungeon.level()
            .mobsMutable()
            .front()
            .get();

    assert(mob);

    const int near =
        adjacentPassable(
            dungeon.level(),
            hero.pos
        );

    assert(near >= 0);

    dungeon.scheduler()
        .freeCell(mob->pos);

    mob->pos = near;

    dungeon.scheduler()
        .occupyCell(mob);

    dungeon.observe();

    Scroll* terror =
        addScroll(
            hero,
            ScrollKind::Terror
        );

    assert(
        dungeon.readScroll(
            terror
        )
    );

    Terror* terrorBuff =
        dynamic_cast<Terror*>(
            mob->findBuff(
                typeid(Terror)
            )
        );

    assert(terrorBuff);
    assert(
        terrorBuff->object ==
        hero.id()
    );

    terrorBuff->detach();

    Scroll* lullaby =
        addScroll(
            hero,
            ScrollKind::Lullaby
        );

    assert(
        dungeon.readScroll(
            lullaby
        )
    );

    assert(
        mob->state() ==
        Mob::State::Sleeping
    );

    Scroll* challenge =
        addScroll(
            hero,
            ScrollKind::Challenge
        );

    assert(
        dungeon.readScroll(
            challenge
        )
    );

    assert(
        mob->target() ==
        hero.pos
    );

    assert(
        mob->hasBuff(
            typeid(Rage)
        )
    );

    const int before =
        mob->HP;

    Scroll* blast =
        addScroll(
            hero,
            ScrollKind::PsionicBlast
        );

    assert(
        dungeon.readScroll(
            blast
        )
    );

    assert(
        hero.hasBuff(
            typeid(Blindness)
        )
    );

    // Mob may die from the blast; if alive it must be damaged and blinded.
    if (mob->isAlive()) {
        assert(mob->HP < before);
        assert(
            mob->hasBuff(
                typeid(Blindness)
            )
        );
    }
}

static void mirrorImageScroll() {
    Dungeon dungeon;

    assert(
        dungeon.startNew(
            HeroClass::Rogue,
            0x160040u
        )
    );

    Hero& hero =
        dungeon.hero();

    const std::size_t before =
        dungeon.level()
            .mobs()
            .size();

    Scroll* mirror =
        addScroll(
            hero,
            ScrollKind::MirrorImage
        );

    assert(
        dungeon.readScroll(
            mirror
        )
    );

    int mirrors = 0;

    for (const auto& mob :
         dungeon.level().mobs()) {

        if (dynamic_cast<
                MirrorImage*
            >(mob.get())) {
            ++mirrors;
        }
    }

    assert(mirrors >= 1);
    assert(mirrors <= 3);

    assert(
        dungeon.level()
            .mobs()
            .size() >
        before
    );
}

static void unknownInventoryCancelConsumes() {
    Dungeon dungeon;

    assert(
        dungeon.startNew(
            HeroClass::Warrior,
            0x160050u
        )
    );

    Scroll* identify =
        addScroll(
            dungeon.hero(),
            ScrollKind::Identify
        );

    assert(!identify->isKnown());

    identify->setKnown();

    assert(
        dungeon.consumeScrollOnCancel(
            identify
        )
    );

    // Pointer is invalid after detach/consume; verify no identify scroll remains.
    Scroll* remaining =
        dungeon.hero()
            .belongings()
            .getItem<Scroll>();

    if (remaining) {
        assert(
            remaining->kind() !=
            ScrollKind::Identify
        );
    }
}

int main() {
    statusHandlerPermutation();
    inventoryLayoutAndFilters();

    identifyAndUpgradeScrolls();
    mappingAndRechargeScrolls();

    terrorLullabyChallengePsionic();
    mirrorImageScroll();

    unknownInventoryCancelConsumes();

    std::cout
        << "PIXEL DUNGEON CONSOLE EDITION v0.16 scroll + inventory tests: OK\n";

    return 0;
}
