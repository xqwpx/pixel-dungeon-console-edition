#include "game/ui/BuffIndicator.h"

#include "game/actors/Char.h"
#include "game/actors/buffs/Amok.h"
#include "game/actors/buffs/Levitation.h"
#include "game/actors/buffs/Ooze.h"
#include "game/actors/buffs/GasesImmunity.h"
#include "game/actors/buffs/Bleeding.h"
#include "game/actors/buffs/Blindness.h"
#include "game/actors/buffs/Burning.h"
#include "game/actors/buffs/Cripple.h"
#include "game/actors/buffs/Frost.h"
#include "game/actors/buffs/Hunger.h"
#include "game/actors/buffs/Invisibility.h"
#include "game/actors/buffs/Light.h"
#include "game/actors/buffs/MindVision.h"
#include "game/actors/buffs/Paralysis.h"
#include "game/actors/buffs/Poison.h"
#include "game/actors/buffs/Roots.h"
#include "game/actors/buffs/Rage.h"
#include "game/actors/buffs/Combo.h"
#include "game/actors/buffs/Fury.h"
#include "game/actors/buffs/SnipersMark.h"
#include "game/actors/buffs/Barkskin.h"
#include "game/actors/buffs/HerbalArmor.h"
#include "game/actors/buffs/HerbalHealing.h"
#include "game/actors/buffs/Shadows.h"
#include "game/actors/buffs/Slow.h"
#include "game/actors/buffs/Terror.h"
#include "game/actors/buffs/Vertigo.h"
#include "game/actors/buffs/Weakness.h"

#include <typeinfo>

namespace pdv {

std::vector<int>
BuffIndicator::iconsFor(
    const Char& ch
) {
    std::vector<int> icons;

    for (Buff* buff :
         ch.activeBuffs()) {

        if (!buff) {
            continue;
        }

        int icon = None;

        if (dynamic_cast<::pdv::Levitation*>(buff)) {
            icon = Levitation;

        } else if (auto* hunger =
                dynamic_cast<::pdv::Hunger*>(buff)) {
            icon =
                hunger->isStarving()
                ? Starvation
                : hunger->isHungry()
                    ? Hunger
                    : None;

        } else if (dynamic_cast<::pdv::MindVision*>(buff)) {
            icon = MindVision;
        } else if (dynamic_cast<::pdv::Burning*>(buff)) {
            icon = Fire;
        } else if (dynamic_cast<::pdv::Poison*>(buff)) {
            icon = Poison;
        } else if (dynamic_cast<::pdv::Paralysis*>(buff)) {
            icon = Paralysis;
        } else if (dynamic_cast<::pdv::Slow*>(buff)) {
            icon = Slow;
        } else if (dynamic_cast<::pdv::Ooze*>(buff)) {
            icon = Ooze;
        } else if (dynamic_cast<::pdv::Amok*>(buff)) {
            icon = Amok;
        } else if (dynamic_cast<::pdv::Terror*>(buff)) {
            icon = Terror;
        } else if (dynamic_cast<::pdv::Roots*>(buff)) {
            icon = Roots;
        } else if (dynamic_cast<::pdv::Shadows*>(buff)) {
            icon = Shadows;
        } else if (dynamic_cast<::pdv::Invisibility*>(buff)) {
            icon = Invisible;
        } else if (dynamic_cast<::pdv::Weakness*>(buff)) {
            icon = Weakness;
        } else if (dynamic_cast<::pdv::Frost*>(buff)) {
            icon = Frost;
        } else if (dynamic_cast<::pdv::Blindness*>(buff)) {
            icon = Blindness;
        } else if (dynamic_cast<::pdv::Light*>(buff)) {
            icon = Light;
        } else if (dynamic_cast<::pdv::Cripple*>(buff)) {
            icon = Cripple;
        } else if (dynamic_cast<::pdv::GasesImmunity*>(buff)) {
            icon = Immunity;
        } else if (dynamic_cast<::pdv::Bleeding*>(buff)) {
            icon = Bleeding;
        } else if (dynamic_cast<::pdv::Vertigo*>(buff)) {
            icon = Vertigo;
        } else if (dynamic_cast<::pdv::Combo*>(buff)) {
            icon = Combo;
        } else if (dynamic_cast<::pdv::Fury*>(buff)) {
            icon = Fury;
        } else if (dynamic_cast<::pdv::SnipersMark*>(buff)) {
            icon = Mark;
        } else if (dynamic_cast<::pdv::Barkskin*>(buff)) {
            icon = Barkskin;
        } else if (dynamic_cast<::pdv::HerbalArmor*>(buff)) {
            icon = Armor;
        } else if (dynamic_cast<::pdv::HerbalHealing*>(buff)) {
            icon = Healing;
        } else if (dynamic_cast<::pdv::Rage*>(buff)) {
            icon = Rage;
        }

        if (icon != None) {
            icons.push_back(icon);
        }
    }

    return icons;
}

} // namespace pdv
