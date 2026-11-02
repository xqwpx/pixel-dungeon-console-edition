#pragma once
#include "game/actors/mobs/Mob.h"
#include <memory>
namespace pdv {
class Weapon;
enum class SpecialMobKind{Wraith,Statue,FetidRat,CursePersonification,Bee};
class SpecialMob:public Mob{
public:
    SpecialMob(SpecialMobKind kind,int depth,bool createEquipment=true);
    ~SpecialMob()override;
    SpecialMobKind kind()const{return kind_;}int depth()const{return depth_;}
    int damageRoll()const override;int attackSkill(const Char&)const override;int dr()const override;
    int attackProc(Char&,int)override;int defenseProc(Char&,int)override;
    bool act()override;void damage(int,const void*)override;void die(const void*)override;void beckon(int)override;
    bool immuneTo(const std::type_info&)const override;bool resistantTo(const std::type_info&)const override;const char*description()const override;
    const Weapon*weapon()const{return weapon_.get();}void restoreWeapon(std::unique_ptr<Weapon>weapon){weapon_=std::move(weapon);}
protected:Char*chooseEnemy()override;float attackDelay()const override;
private:SpecialMobKind kind_;int depth_=1;std::unique_ptr<Weapon>weapon_;
};
}
