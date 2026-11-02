#include "game/items/potions/Potion.h"

#include "game/Dungeon.h"
#include "game/actors/Hero.h"
#include "game/items/ItemSpriteSheet.h"
#include "game/items/ItemStatusHandler.h"
#include "game/Progression.h"
#include "core/Localization.h"

#include "game/items/potions/PotionOfHealing.h"
#include "game/items/potions/PotionOfExperience.h"
#include "game/items/potions/PotionOfToxicGas.h"
#include "game/items/potions/PotionOfLiquidFlame.h"
#include "game/items/potions/PotionOfStrength.h"
#include "game/items/potions/PotionOfParalyticGas.h"
#include "game/items/potions/PotionOfLevitation.h"
#include "game/items/potions/PotionOfMindVision.h"
#include "game/items/potions/PotionOfPurity.h"
#include "game/items/potions/PotionOfInvisibility.h"
#include "game/items/potions/PotionOfMight.h"
#include "game/items/potions/PotionOfFrost.h"

#include <array>

namespace pdv {

namespace {

ItemStatusHandler gPotionStatus;

const std::array<std::string, 12> kColors = {{
    "turquoise", "crimson", "azure", "jade", "golden", "magenta",
    "charcoal", "ivory", "amber", "bistre", "indigo", "silver"
}};

const std::array<int, 12> kImages = {{
    ItemSpriteSheet::POTION_TURQUOISE,
    ItemSpriteSheet::POTION_CRIMSON,
    ItemSpriteSheet::POTION_AZURE,
    ItemSpriteSheet::POTION_JADE,
    ItemSpriteSheet::POTION_GOLDEN,
    ItemSpriteSheet::POTION_MAGENTA,
    ItemSpriteSheet::POTION_CHARCOAL,
    ItemSpriteSheet::POTION_IVORY,
    ItemSpriteSheet::POTION_AMBER,
    ItemSpriteSheet::POTION_BISTRE,
    ItemSpriteSheet::POTION_INDIGO,
    ItemSpriteSheet::POTION_SILVER
}};

} // namespace

Potion::Potion() {
    stackable_ = true;
}

ItemStatusHandler& Potion::handler() {
    return gPotionStatus;
}

void Potion::initColors() {
    handler().init(kColors, kImages);
}
uint16_t Potion::knownMask(){return handler().knownMask();}
void Potion::restoreKnownMask(uint16_t mask){handler().restoreKnownMask(mask);}

int Potion::statusIndex(const Potion& potion) {
    if (dynamic_cast<const PotionOfHealing*>(&potion)) return 0;
    if (dynamic_cast<const PotionOfExperience*>(&potion)) return 1;
    if (dynamic_cast<const PotionOfToxicGas*>(&potion)) return 2;
    if (dynamic_cast<const PotionOfLiquidFlame*>(&potion)) return 3;
    if (dynamic_cast<const PotionOfMight*>(&potion)) return 10;
    if (dynamic_cast<const PotionOfStrength*>(&potion)) return 4;
    if (dynamic_cast<const PotionOfParalyticGas*>(&potion)) return 5;
    if (dynamic_cast<const PotionOfLevitation*>(&potion)) return 6;
    if (dynamic_cast<const PotionOfMindVision*>(&potion)) return 7;
    if (dynamic_cast<const PotionOfPurity*>(&potion)) return 8;
    if (dynamic_cast<const PotionOfInvisibility*>(&potion)) return 9;
    if (dynamic_cast<const PotionOfFrost*>(&potion)) return 11;
    return 0;
}

bool Potion::isKnown() const {
    return handler().isKnown(statusIndex(*this));
}

void Potion::setKnown() {
    handler().know(statusIndex(*this));
    if (handler().allKnown()) Progression::instance().notePotionIdentified();
}

Item& Potion::identify() {
    setKnown();
    return *this;
}

int Potion::price() const {
    if (!isKnown()) return 20 * quantity();
    int unit = 20;
    if (dynamic_cast<const PotionOfHealing*>(this)) unit=30;
    else if (dynamic_cast<const PotionOfExperience*>(this)) unit=80;
    else if (dynamic_cast<const PotionOfToxicGas*>(this) || dynamic_cast<const PotionOfLiquidFlame*>(this) || dynamic_cast<const PotionOfParalyticGas*>(this) || dynamic_cast<const PotionOfInvisibility*>(this)) unit=40;
    else if (dynamic_cast<const PotionOfMight*>(this)) unit=200;
    else if (dynamic_cast<const PotionOfStrength*>(this)) unit=100;
    else if (dynamic_cast<const PotionOfLevitation*>(this) || dynamic_cast<const PotionOfMindVision*>(this)) unit=35;
    else if (dynamic_cast<const PotionOfPurity*>(this) || dynamic_cast<const PotionOfFrost*>(this)) unit=50;
    return unit * quantity();
}

int Potion::image() const {
    return handler().image(statusIndex(*this));
}

std::string Potion::color() const {
    return handler().label(statusIndex(*this));
}

std::string Potion::displayName() const {
    return isKnown() ? Item::displayName() : color() + " " + Localization::text("potion");
}

std::string Potion::description() const {
    if (!isKnown()) {
        return Localization::text("A magical potion. Its effect is unknown until you drink it.");
    }

    if (dynamic_cast<const PotionOfHealing*>(this)) {
        return Localization::text("Restores all health and removes poison, bleeding, weakness, and crippling effects.");
    }
    if (dynamic_cast<const PotionOfExperience*>(this)) {
        return Localization::text("Grants enough experience to reach your next hero level.");
    }
    if (dynamic_cast<const PotionOfToxicGas*>(this)) {
        return Localization::text("Releases a large cloud of toxic gas when shattered.");
    }
    if (dynamic_cast<const PotionOfLiquidFlame*>(this)) {
        return Localization::text("Creates a patch of fire when shattered.");
    }
    if (dynamic_cast<const PotionOfStrength*>(this)) {
        return Localization::text("Permanently increases your strength by one point.");
    }
    if (dynamic_cast<const PotionOfParalyticGas*>(this)) {
        return Localization::text("Releases a large cloud of paralysing gas when shattered.");
    }
    if (dynamic_cast<const PotionOfLevitation*>(this)) {
        return Localization::text("Lets you levitate for a time, safely passing over hazards.");
    }
    if (dynamic_cast<const PotionOfMindVision*>(this)) {
        return Localization::text("Lets you sense creatures through walls for a time.");
    }
    if (dynamic_cast<const PotionOfPurity*>(this)) {
        return Localization::text("Protects you from harmful gases for a time and clears nearby toxic or paralytic gas when shattered.");
    }
    if (dynamic_cast<const PotionOfInvisibility*>(this)) {
        return Localization::text("Makes you invisible to enemies for a time.");
    }
    if (dynamic_cast<const PotionOfMight*>(this)) {
        return Localization::text("Permanently increases strength and maximum health by five.");
    }
    if (dynamic_cast<const PotionOfFrost*>(this)) {
        return Localization::text("Freezes nearby creatures and extinguishes nearby fire when shattered.");
    }

    return Localization::text("A magical potion.");
}

void Potion::apply(Dungeon& dungeon, Hero& hero) {
    shatter(dungeon, hero.pos);
}

void Potion::shatter(Dungeon&, int) {}

void Potion::onThrow(Dungeon& dungeon, Hero& hero, int cell) {
    if (cell == hero.pos) {
        apply(dungeon, hero);
    } else {
        shatter(dungeon, cell);
    }
}

} // namespace pdv
