#include "game/items/weapon/Enchantment.h"
#include "core/Localization.h"
#include "game/items/weapon/Weapon.h"
#include "game/actors/Char.h"
#include "game/actors/Hero.h"
#include "game/actors/mobs/SpecialMob.h"
#include "game/actors/mobs/ZoneMob.h"
#include "game/actors/mobs/Goo.h"
#include "game/actors/mobs/Skeleton.h"
#include "game/actors/buffs/Buff.h"
#include "game/actors/buffs/Burning.h"
#include "game/actors/buffs/Poison.h"
#include "game/actors/buffs/Paralysis.h"
#include "game/actors/buffs/Slow.h"
#include "game/actors/buffs/Terror.h"
#include "game/actors/buffs/Vertigo.h"
#include "game/levels/Level.h"
#include "game/items/rings/Ring.h"
#include "game/Progression.h"
#include "game/utils/Random.h"
#include <algorithm>
#include <vector>

namespace pdv {
namespace {
void shockHit(Char&ch,int damage,Level*level,ActorScheduler*scheduler,std::vector<Char*>&affected,int depth){
    if(damage<1||depth>=20)return;
    affected.push_back(&ch);
    ch.damage(level&&level->water(ch.pos)&&!ch.flying?damage*2:damage,nullptr);
    if(!level||!scheduler)return;
    std::vector<Char*>neighbours;
    for(int offset:Level::Neighbours8){Char*next=scheduler->findChar(ch.pos+offset);if(next&&std::find(affected.begin(),affected.end(),next)==affected.end())neighbours.push_back(next);}
    if(!neighbours.empty()){Char*next=Random::element(neighbours);shockHit(*next,Random::Int(damage/2,damage),level,scheduler,affected,depth+1);}
}
class OriginalEnchantment final : public Enchantment {
public:
    explicit OriginalEnchantment(EnchantmentKind kind) : kind_(kind) {}
    EnchantmentKind kind() const override { return kind_; }
    std::unique_ptr<Enchantment> clone() const override {
        return std::unique_ptr<Enchantment>(new OriginalEnchantment(kind_));
    }
    std::string name(const std::string& n) const override {
        switch (kind_) {
            case EnchantmentKind::Fire: return Localization::text("blazing") + " " + n;
            case EnchantmentKind::Poison: return Localization::text("venomous") + " " + n;
            case EnchantmentKind::Death: return Localization::text("grim") + " " + n;
            case EnchantmentKind::Paralysis: return Localization::text("stunning") + " " + n;
            case EnchantmentKind::Leech: return Localization::text("vampiric") + " " + n;
            case EnchantmentKind::Slow: return Localization::text("chilling") + " " + n;
            case EnchantmentKind::Shock: return Localization::text("shocking") + " " + n;
            case EnchantmentKind::Instability: return Localization::text("unstable") + " " + n;
            case EnchantmentKind::Horror: return Localization::text("eldritch") + " " + n;
            case EnchantmentKind::Luck: return Localization::text("lucky") + " " + n;
            case EnchantmentKind::Tempering: return Localization::text("tempered") + " " + n;
        }
        return n;
    }
    bool proc(Weapon& weapon, Char& attacker, Char& defender, int damage) override {
        const int level = std::max(0, weapon.effectiveLevel());
        switch (kind_) {
            case EnchantmentKind::Fire:
                if (Random::Int(level + 3) < 2) return false;
                if (const ZoneMob* zone = dynamic_cast<const ZoneMob*>(&defender)) {
                    if (zone->kind() == ZoneMobKind::Elemental) return false;
                }
                if (Random::Int(2) == 0) Buff::affect<Burning>(defender)->reignite(defender);
                defender.damage(Random::Int(1, level + 2), this); return true;
            case EnchantmentKind::Poison:
                if (Random::Int(level + 3) < 2) return false;
                Buff::affect<Poison>(defender)->set(Poison::durationFactor(defender) * (level + 1)); return true;
            case EnchantmentKind::Death:
                if (Random::Int(level + 100) < 92) return false;
                if (const SpecialMob* special = dynamic_cast<const SpecialMob*>(&defender)) {
                    if (special->kind() == SpecialMobKind::Wraith || special->kind() == SpecialMobKind::Statue || special->kind() == SpecialMobKind::CursePersonification) return false;
                }
                if (const ZoneMob* zone = dynamic_cast<const ZoneMob*>(&defender)) {
                    if (zone->kind() == ZoneMobKind::Undead || zone->kind() == ZoneMobKind::RottingFist || zone->kind() == ZoneMobKind::BurningFist || zone->kind() == ZoneMobKind::Eye || zone->kind() == ZoneMobKind::Warlock) return false;
                }
                if (const ZoneBoss* boss = dynamic_cast<const ZoneBoss*>(&defender)) {
                    if (boss->kind() == ZoneBossKind::Tengu || boss->kind() == ZoneBossKind::DM300 || boss->kind() == ZoneBossKind::King || boss->kind() == ZoneBossKind::Yog) return false;
                }
                if (dynamic_cast<const Goo*>(&defender)) return false;
                if (dynamic_cast<const Skeleton*>(&defender)) return false;
                defender.damage(defender.HP, this);
                if (!defender.isAlive() && dynamic_cast<Hero*>(&attacker)) Progression::instance().noteGrimWeapon();
                return true;
            case EnchantmentKind::Paralysis:
                if (Random::Int(level + 8) < 7) return false;
                Buff::prolong<Paralysis>(defender, Random::Float(1.0f, 1.5f + level)); return true;
            case EnchantmentKind::Leech: {
                if (const SpecialMob* special = dynamic_cast<const SpecialMob*>(&defender)) {
                    if (special->kind() == SpecialMobKind::Statue) return false;
                }
                if (const ZoneMob* zone = dynamic_cast<const ZoneMob*>(&defender)) {
                    if (zone->kind() == ZoneMobKind::Bat || zone->kind() == ZoneMobKind::Succubus || zone->kind() == ZoneMobKind::Eye || zone->kind() == ZoneMobKind::Scorpio || zone->kind() == ZoneMobKind::Acidic) return false;
                }
                const int maxValue = damage * (level + 2) / (level + 6);
                const int value = std::min(Random::IntRange(0, maxValue), attacker.HT - attacker.HP);
                if (value <= 0) return false;
                attacker.HP += value; return true;
            }
            case EnchantmentKind::Slow:
                if (Random::Int(level + 4) < 3) return false;
                Buff::prolong<Slow>(defender, Random::Float(1.0f, 1.5f + level)); return true;
            case EnchantmentKind::Shock:
                if (Random::Int(level + 4) < 3) return false;
                {std::vector<Char*>affected{&attacker};shockHit(defender,Random::Int(1,damage/2),defender.levelContext(),attacker.attachedScheduler(),affected,0);}return true;
            case EnchantmentKind::Instability: {
                std::unique_ptr<Enchantment> effect = Enchantment::random();
                return effect->proc(weapon, attacker, defender, damage);
            }
            case EnchantmentKind::Horror:
                if (Random::Int(level + 5) < 4) return false;
                if (dynamic_cast<Hero*>(&defender)) Buff::affect<Vertigo>(defender, Vertigo::Duration*elementsDurationFactor(defender));
                else { Terror* terror = Buff::affect<Terror>(defender, Terror::Duration); terror->object = attacker.id(); }
                return true;
            case EnchantmentKind::Luck: {
                int result = damage;
                for (int i = 1; i <= level + 1; ++i) result = std::max(result, attacker.damageRoll() - i);
                if (result <= damage) return false;
                defender.damage(result - damage, this); return true;
            }
            case EnchantmentKind::Tempering:
                weapon.polish(); return true;
        }
        return false;
    }
private:
    EnchantmentKind kind_;
};
}

int Enchantment::weight(EnchantmentKind kind) {
    static const int weights[] = {10,10,1,2,1,2,6,3,2,2,3};
    return weights[static_cast<int>(kind) - 1];
}
std::unique_ptr<Enchantment> Enchantment::create(EnchantmentKind kind) {
    return std::unique_ptr<Enchantment>(new OriginalEnchantment(kind));
}
std::unique_ptr<Enchantment> Enchantment::random() {
    int total = 0; for (int i=1;i<=11;++i) total += weight(static_cast<EnchantmentKind>(i));
    int roll = Random::Int(total);
    for (int i=1;i<=11;++i) { roll -= weight(static_cast<EnchantmentKind>(i)); if (roll < 0) return create(static_cast<EnchantmentKind>(i)); }
    return create(EnchantmentKind::Fire);
}
} // namespace pdv
