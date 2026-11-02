#include <cassert>
#include <cmath>
#include <iostream>

#include "game/Dungeon.h"
#include "game/actors/Hero.h"
#include "game/actors/mobs/Rat.h"
#include "game/items/rings/RingOfShadows.h"
#include "game/items/wands/WandOfMagicMissile.h"
#include "game/items/weapon/missiles/Dart.h"
#include "game/items/weapon/missiles/Boomerang.h"
#include "game/items/EquipmentCatalog.h"
#include "game/actors/buffs/Burning.h"
#include "game/actors/buffs/Cripple.h"
#include "game/actors/buffs/Paralysis.h"
#include "game/actors/buffs/Bleeding.h"
#include "game/mechanics/Ballistica.h"
#include "game/ui/QuickSlot.h"
#include "game/levels/Terrain.h"
#include "game/utils/Random.h"

using namespace pdv;

static void startingKits(){
    Hero warrior; warrior.initialize(HeroClass::Warrior,100);
    Dart* wd=warrior.belongings().getItem<Dart>();
    assert(wd && wd->quantity()==8);
    assert(warrior.primaryQuickSlot().kind()==QuickSlotKind::Dart);
    assert(warrior.primaryQuickSlot().select(warrior)==wd);

    Hero mage; mage.initialize(HeroClass::Mage,100);
    auto* wand=mage.belongings().getItem<WandOfMagicMissile>();
    assert(wand && wand->curCharges()==3 && wand->maxCharges()==3);
    assert(mage.primaryQuickSlot().kind()==QuickSlotKind::WandOfMagicMissile);
    assert(mage.primaryQuickSlot().select(mage)==wand);
    assert(std::fabs(wand->rechargeDelay(mage)-40.0f)<0.001f);
    wand->upgrade(3);
    assert(wand->level()==3);
    assert(wand->maxCharges()==6);
    assert(std::fabs(wand->rechargeDelay(mage)-20.0f)<0.001f);

    Hero rogue; rogue.initialize(HeroClass::Rogue,100);
    auto* ring=dynamic_cast<RingOfShadows*>(rogue.belongings().ring1());
    assert(ring && ring->level()==1);
    assert(rogue.stealth()==1);
    Dart* rd=rogue.belongings().getItem<Dart>();
    assert(rd && rd->quantity()==8);
    assert(rogue.primaryQuickSlot().select(rogue)==rd);

    Hero huntress; huntress.initialize(HeroClass::Huntress,100);
    auto* boom=huntress.belongings().getItem<Boomerang>();
    assert(boom);
    assert(huntress.primaryQuickSlot().kind()==QuickSlotKind::Boomerang);
    assert(huntress.primaryQuickSlot().select(huntress)==boom);
}

static int clearLineToRat(Dungeon& dungeon, int distance=3){
    Hero& hero=dungeon.hero();
    int hx=hero.pos%Level::Width;
    int sx = (hx+distance+2 < Level::Width-1) ? 1 : -1;
    for(int i=1;i<=distance+1;i++) dungeon.level().setRaw(hero.pos+i*sx, Terrain::EMPTY);
    dungeon.level().buildFlagMaps();
    Rat* rat=nullptr;
    for(auto& mob:dungeon.level().mobsMutable())if((rat=dynamic_cast<Rat*>(mob.get())))break;
    assert(rat);
    dungeon.scheduler().freeCell(rat->pos);
    rat->pos=hero.pos+distance*sx;
    rat->HP=rat->HT=8;
    rat->setState(Mob::State::Sleeping);
    dungeon.scheduler().occupyCell(rat);
    dungeon.observe();
    return rat->pos;
}

static void ballisticaStopsAtChars(){
    Dungeon dungeon;
    assert(dungeon.startNew(HeroClass::Warrior,0x1111));
    int ratCell=clearLineToRat(dungeon,3);
    int step = ratCell>dungeon.hero().pos ? 1 : -1;
    int result=Ballistica::cast(dungeon,dungeon.hero().pos,ratCell+step,false,true);
    assert(result==ratCell);
}

static void missileClassProficiency(){
    Rat target; target.pos=103;
    Dart dart(8);

    Hero warrior; warrior.initialize(HeroClass::Warrior,100);
    warrior.shoot(target,dart); // establish ranged path once; random outcome irrelevant
    // Directly inspect factor: STR11 -> base encumbrance -1, warrior missile +3 => 2.
    assert(std::fabs(dart.accuracyFactor(warrior)-(1.0f/2.25f))<0.001f);

    Hero huntress; huntress.initialize(HeroClass::Huntress,100);
    assert(std::fabs(dart.accuracyFactor(huntress)-1.0f)<0.001f);
    assert(std::fabs(dart.speedFactor(huntress)-1.0f)<0.001f);
}

static void dartConsumesOrDrops(){
    Dungeon dungeon;
    assert(dungeon.startNew(HeroClass::Warrior,0x2222));
    int ratCell=clearLineToRat(dungeon,3);
    Dart* dart=dungeon.hero().belongings().getItem<Dart>();
    assert(dart && dart->quantity()==8);
    Random::seed(3);
    assert(dungeon.throwMissileAt(dart,ratCell));
    Dart* remaining=dungeon.hero().belongings().getItem<Dart>();
    assert(remaining && remaining->quantity()==7);
}

static void boomerangReturns(){
    Dungeon dungeon;
    assert(dungeon.startNew(HeroClass::Huntress,0x3333));
    int ratCell=clearLineToRat(dungeon,3);
    auto* boom=dungeon.hero().belongings().getItem<Boomerang>();
    assert(boom);
    assert(dungeon.throwMissileAt(boom,ratCell));
    auto* returned=dungeon.hero().belongings().getItem<Boomerang>();
    assert(returned==boom);
    assert(dungeon.hero().primaryQuickSlot().select(dungeon.hero())==boom);
}

static void magicMissileUsesChargeAndDamages(){
    Dungeon dungeon;
    assert(dungeon.startNew(HeroClass::Mage,0x4444));
    int ratCell=clearLineToRat(dungeon,3);
    auto* wand=dungeon.hero().belongings().getItem<WandOfMagicMissile>();
    auto* rat=dynamic_cast<Rat*>(dungeon.mobAt(ratCell));
    assert(wand && rat);
    int hp=rat->HP;
    Random::seed(7);
    assert(dungeon.zapWandAt(wand,ratCell));
    assert(wand->curCharges()==2);
    assert(rat->HP < hp);
    assert(hp-rat->HP >= 1 && hp-rat->HP <= 5);
}

static void catalogMissileEffects(){
    Hero hero;hero.initialize(HeroClass::Huntress,100);Rat target;target.pos=103;target.HP=target.HT=100;
    CatalogMissile javelin(MissileKind::Javelin);javelin.proc(hero,target,5);assert(target.hasBuff(typeid(Cripple)));
    CatalogMissile curare(MissileKind::CurareDart);curare.proc(hero,target,2);assert(target.hasBuff(typeid(Paralysis)));
    CatalogMissile incendiary(MissileKind::IncendiaryDart);incendiary.proc(hero,target,2);assert(target.hasBuff(typeid(Burning)));
    CatalogMissile tomahawk(MissileKind::Tomahawk);tomahawk.proc(hero,target,7);Bleeding*bleeding=dynamic_cast<Bleeding*>(target.findBuff(typeid(Bleeding)));assert(bleeding&&bleeding->level()==7);
    // DLY 0.5, increased by one effective point of encumbrance for a
    // starting Huntress: 0.5 * 1.2, matching Weapon.java.
    CatalogMissile shuriken(MissileKind::Shuriken);assert(std::fabs(shuriken.speedFactor(hero)-0.6f)<0.001f);
}

int main(){
    startingKits();
    ballisticaStopsAtChars();
    missileClassProficiency();
    dartConsumesOrDrops();
    boomerangReturns();
    magicMissileUsesChargeAndDamages();
    catalogMissileEffects();
    std::cout << "PIXEL DUNGEON CONSOLE EDITION v0.11 ranged + magic parity tests: OK\n";
    return 0;
}
