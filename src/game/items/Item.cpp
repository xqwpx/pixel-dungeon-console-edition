#include "game/items/Item.h"
#include "core/Localization.h"
#include <typeinfo>
#include <sstream>

namespace pdv {
std::string Item::displayName() const { return Localization::itemName(name_); }
const char* itemActionName(ItemAction a){
switch(a){case ItemAction::Equip:return Localization::action("Equip");case ItemAction::Unequip:return Localization::action("Unequip");case ItemAction::Drop:return Localization::action("Drop");case ItemAction::Throw:return Localization::action("Throw");case ItemAction::Use:return Localization::action("Use");case ItemAction::Drink:return Localization::action("Drink");case ItemAction::Read:return Localization::action("Read");case ItemAction::Eat:return Localization::action("Eat");case ItemAction::Zap:return Localization::action("Zap");case ItemAction::QuickSlot:return Localization::action("Quick slot");case ItemAction::Gladiator:return Localization::action("Gladiator");case ItemAction::Berserker:return Localization::action("Berserker");case ItemAction::Battlemage:return Localization::action("Battlemage");case ItemAction::Warlock:return Localization::action("Warlock");case ItemAction::Assassin:return Localization::action("Assassin");case ItemAction::Freerunner:return Localization::action("Freerunner");case ItemAction::Sniper:return Localization::action("Sniper");case ItemAction::Warden:return Localization::action("Warden");case ItemAction::HeroicLeap:return Localization::action("Heroic leap");case ItemAction::MoltenEarth:return Localization::action("Molten earth");case ItemAction::SmokeBomb:return Localization::action("Smoke bomb");case ItemAction::SpectralBlades:return Localization::action("Spectral blades");case ItemAction::BalanceSpeed:return Localization::action("Balance for speed");case ItemAction::BalanceAccuracy:return Localization::action("Balance for accuracy");case ItemAction::Mine:return Localization::action("Mine");case ItemAction::Shatter:return Localization::action("Shatter");} return "";
}
std::vector<ItemAction> Item::actions(const Hero&) const { return {ItemAction::Drop,ItemAction::Throw}; }
std::string Item::info() const { std::ostringstream out; out<<displayName(); if(levelKnown_) out<<(level_>=0?" +":" ")<<level_; if(quantity_>1) out<<" x"<<quantity_; if(cursedKnown_) out<<" | "<<Localization::text(cursed_?"cursed":"not cursed"); return out.str(); }

void Item::use() {
    if (level_ > 0 && !isBroken()) {
        durability_ = durability() - 1;
    }
}

Item& Item::identify() {
    levelKnown_ = true;
    cursedKnown_ = true;
    return *this;
}

int Item::priceWithState(int price) const {
    if (cursed_ && cursedKnown_) price /= 2;
    if (levelKnown_) {
        if (level_ > 0) {
            price *= level_ + 1;
            if (isBroken()) price /= 2;
        } else if (level_ < 0) {
            price /= 1 - level_;
        }
    }
    return std::max(1, price);
}

bool Item::sameKind(const Item& other) const {
    return typeid(*this) == typeid(other);
}

} // namespace pdv
