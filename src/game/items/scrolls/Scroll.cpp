#include "game/items/scrolls/Scroll.h"

#include "game/items/ItemSpriteSheet.h"
#include "game/items/ItemStatusHandler.h"
#include "game/Progression.h"
#include "core/Localization.h"

#include <array>

namespace pdv {

namespace {

ItemStatusHandler gScrollStatus;

const std::array<std::string, 12> kRunes = {{
    "KAUNAN", "SOWILO", "LAGUZ", "YNGVI", "GYFU", "RAIDO",
    "ISAZ", "MANNAZ", "NAUDIZ", "BERKANAN", "ODAL", "TIWAZ"
}};

const std::array<int, 12> kImages = {{
    ItemSpriteSheet::SCROLL_KAUNAN,
    ItemSpriteSheet::SCROLL_SOWILO,
    ItemSpriteSheet::SCROLL_LAGUZ,
    ItemSpriteSheet::SCROLL_YNGVI,
    ItemSpriteSheet::SCROLL_GYFU,
    ItemSpriteSheet::SCROLL_RAIDO,
    ItemSpriteSheet::SCROLL_ISAZ,
    ItemSpriteSheet::SCROLL_MANNAZ,
    ItemSpriteSheet::SCROLL_NAUDIZ,
    ItemSpriteSheet::SCROLL_BERKANAN,
    ItemSpriteSheet::SCROLL_ODAL,
    ItemSpriteSheet::SCROLL_TIWAZ
}};

} // namespace

void Scroll::initLabels() {
    gScrollStatus.init(kRunes, kImages);
}
uint16_t Scroll::knownMask(){return gScrollStatus.knownMask();}
void Scroll::restoreKnownMask(uint16_t mask){gScrollStatus.restoreKnownMask(mask);}

Scroll::Scroll(ScrollKind kind)
    : kind_(kind) {
    name_ = kindName(kind);
    stackable_ = true;
}

int Scroll::statusIndex(ScrollKind kind) {
    switch (kind) {
        case ScrollKind::Identify:      return 0;
        case ScrollKind::MagicMapping:  return 1;
        case ScrollKind::Recharging:    return 2;
        case ScrollKind::RemoveCurse:   return 3;
        case ScrollKind::Teleportation: return 4;
        case ScrollKind::Challenge:     return 5;
        case ScrollKind::Terror:        return 6;
        case ScrollKind::Lullaby:       return 7;
        case ScrollKind::PsionicBlast:  return 8;
        case ScrollKind::MirrorImage:   return 9;
        case ScrollKind::Upgrade:       return 10;
        case ScrollKind::Enchantment:   return 11;
        case ScrollKind::WipeOut:       return -1;
    }

    return 0;
}

std::string Scroll::rune() const {
    return gScrollStatus.label(statusIndex(kind_));
}

bool Scroll::isKnown() const {
    if(kind_==ScrollKind::WipeOut)return true;
    return gScrollStatus.isKnown(statusIndex(kind_));
}

void Scroll::setKnown() {
    if(kind_==ScrollKind::WipeOut)return;
    gScrollStatus.know(statusIndex(kind_));
    if(gScrollStatus.allKnown()) Progression::instance().noteScrollIdentified();
}

Item& Scroll::identify() {
    setKnown();
    return *this;
}

int Scroll::price() const {
    if (!isKnown()) return 15 * quantity();
    int unit=15;
    switch(kind_){
        case ScrollKind::Identify: case ScrollKind::RemoveCurse: unit=30;break;
        case ScrollKind::MagicMapping: unit=25;break;
        case ScrollKind::Teleportation: case ScrollKind::Recharging: unit=40;break;
        case ScrollKind::Terror: case ScrollKind::Lullaby: unit=50;break;
        case ScrollKind::PsionicBlast: unit=80;break;
        case ScrollKind::WipeOut: unit=100;break;
        default: break;
    }
    return unit * quantity();
}

int Scroll::image() const {
    if(kind_==ScrollKind::WipeOut)return 117;
    return gScrollStatus.image(statusIndex(kind_));
}

std::string Scroll::displayName() const {
    return isKnown() ? Item::displayName() : Localization::text("scroll") + " \"" + rune() + "\"";
}

std::string Scroll::description() const {
    if (!isKnown()) {
        return Localization::text("This parchment is covered with indecipherable writing. Its magical effect is unknown.");
    }

    switch (kind_) {
        case ScrollKind::Identify:
            return Localization::text("Permanently reveals all the properties and secrets of a single item.");
        case ScrollKind::Teleportation:
            return Localization::text("Instantly transports you to a random location on this floor. It can save you from danger, but may also place you somewhere worse.");
        case ScrollKind::RemoveCurse:
            return Localization::text("Strips cursed weapons, armor, rings, and carried items of the evil magic that prevents their removal.");
        case ScrollKind::Recharging:
            return Localization::text("Releases magical power that restores all of your wands to full charge.");
        case ScrollKind::MagicMapping:
            return Localization::text("Reveals the complete layout of this floor and all hidden secrets. The locations of creatures and items remain unknown.");
        case ScrollKind::Challenge:
            return Localization::text("Unleashes a challenging roar that alerts all enemies to your location and enrages those in sight.");
        case ScrollKind::Terror:
            return Localization::text("Overwhelms all visible enemies with terror, causing them to flee. Attacking a fleeing enemy dispels the effect.");
        case ScrollKind::Lullaby:
            return Localization::text("Puts all visible creatures into a deep sleep, giving you a chance to flee or make a surprise attack.");
        case ScrollKind::PsionicBlast:
            return Localization::text("Channels destructive energy to heavily damage and blind visible enemies. The flash also temporarily blinds you.");
        case ScrollKind::MirrorImage:
            return Localization::text("Creates up to three illusory copies of you that will chase and fight your enemies.");
        case ScrollKind::Upgrade:
            return Localization::text("Upgrades one item, improving its quality and lifting its curse. Weapons, armor, wands, and rings all become more powerful.");
        case ScrollKind::Enchantment:
            return Localization::text("Imbues a weapon with a random enchantment or inscribes armor with a random glyph, granting it special power.");
        case ScrollKind::WipeOut:
            return Localization::text("Unleashes the dungeon spirits' wrath, destroying ordinary hostile enemies on this floor. Powerful creatures may resist it.");
    }

    return Localization::text("A magical scroll.");
}

bool Scroll::requiresInventoryTarget() const {
    return kind_ == ScrollKind::Identify ||
           kind_ == ScrollKind::Upgrade ||
           kind_ == ScrollKind::Enchantment;
}

const char* Scroll::selectionTitle() const {
    switch (kind_) {
        case ScrollKind::Identify:
            return "Select an item to identify";
        case ScrollKind::Upgrade:
            return "Select an item to upgrade";
        case ScrollKind::Enchantment:
            return "Select an enchantable item";
        case ScrollKind::WipeOut:return "";
        default:
            return "Select an item";
    }
}

std::unique_ptr<Item> Scroll::clone() const {
    return std::unique_ptr<Item>(new Scroll(*this));
}

const char* Scroll::kindName(ScrollKind kind) {
    switch (kind) {
        case ScrollKind::Identify:      return "Scroll of Identify";
        case ScrollKind::Teleportation: return "Scroll of Teleportation";
        case ScrollKind::RemoveCurse:   return "Scroll of Remove Curse";
        case ScrollKind::Recharging:    return "Scroll of Recharging";
        case ScrollKind::MagicMapping:  return "Scroll of Magic Mapping";
        case ScrollKind::Challenge:     return "Scroll of Challenge";
        case ScrollKind::Terror:        return "Scroll of Terror";
        case ScrollKind::Lullaby:       return "Scroll of Lullaby";
        case ScrollKind::PsionicBlast:  return "Scroll of Psionic Blast";
        case ScrollKind::MirrorImage:   return "Scroll of Mirror Image";
        case ScrollKind::Upgrade:       return "Scroll of Upgrade";
        case ScrollKind::Enchantment:   return "Scroll of Enchantment";
        case ScrollKind::WipeOut:       return "Scroll of Wipe Out";
    }

    return "Scroll";
}

} // namespace pdv
