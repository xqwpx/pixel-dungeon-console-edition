#include <cassert>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <memory>
#include <vector>

#include "game/Dungeon.h"
#include "game/items/Gold.h"
#include "game/items/food/Pasty.h"
#include "game/items/potions/PotionOfHealing.h"
#include "game/items/scrolls/Scroll.h"
#include "game/levels/Terrain.h"
#include "game/items/ConsumableCatalog.h"
#include "game/actors/blobs/Blob.h"
#include "game/actors/buffs/Buff.h"
#include "game/actors/buffs/Poison.h"
#include "game/actors/buffs/Combo.h"
#include "game/actors/buffs/Charm.h"
#include "game/actors/mobs/Mob.h"
#include "game/actors/mobs/Goo.h"
#include "game/actors/mobs/ZoneMob.h"
#include "game/items/SpecialItems.h"
#include "game/items/weapon/Weapon.h"

using namespace pdv;

int main(){
    const std::string path="save_game_test.dat";
    std::remove(path.c_str());std::remove((path+".tmp").c_str());
    Dungeon source;assert(source.startNew(HeroClass::Rogue,0x51A7E123u));
    source.hero().setSubClass(HeroSubClass::Assassin);
    Weapon*sourceWeapon=dynamic_cast<Weapon*>(source.hero().belongings().weapon());assert(sourceWeapon);sourceWeapon->imbue(Weapon::Imbue::Accuracy);
    source.addGold(137);source.hero().HP=source.hero().HT-3;source.hero().increaseStrength(2);source.hero().earnExp(10);assert(source.hero().spendUpgrade(2));source.hunger().restoreLevel(173.0f);
    source.level().visitedMutable()[77]=true;source.level().mappedMutable()[88]=true;source.level().setRaw(99,Terrain::GRASS);source.level().buildFlagMaps();
    assert(source.level().plant(SeedKind::Sungrass,99));
    assert(source.level().seedBlob(BlobKind::Fire,99,7));
    Poison*poison=Buff::affect<Poison>(source.hero());assert(poison);poison->set(13.0f);
    Combo*combo=Buff::affect<Combo>(source.hero());assert(combo);combo->hit(3);combo->hit(4);
    Charm*charm=Buff::affect<Charm>(source.hero(),6.0f);assert(charm);charm->object=77;
    assert(!source.level().mobs().empty());Poison*mobPoison=Buff::affect<Poison>(*source.level().mobs().front());assert(mobPoison);mobPoison->set(9.0f);
    std::unique_ptr<Goo>goo(new Goo());goo->pos=100;goo->restorePumpedUp(true);source.level().addMob(std::move(goo));
    std::unique_ptr<ZoneBoss>tengu(new ZoneBoss(ZoneBossKind::Tengu));tengu->pos=101;tengu->restoreBossState(true,true,2);source.level().addMob(std::move(tengu));
    assert(source.hero().belongings().backpack().collect(std::unique_ptr<Item>(new Pasty())));
    std::unique_ptr<SpecialItem>vial(new SpecialItem(SpecialKind::DewVial));vial->quantity(6);assert(source.hero().belongings().backpack().collect(std::move(vial)));
    std::unique_ptr<PotionOfHealing> healing(new PotionOfHealing());healing->quantity(2);healing->setKnown();assert(source.hero().belongings().backpack().collect(std::move(healing)));
    Scroll consumedKnowledge(ScrollKind::Terror);consumedKnowledge.setKnown();
    const int heroPos=source.hero().pos;const std::size_t mobs=source.level().mobs().size();const std::size_t heaps=source.level().heaps().size();
    assert(source.save(path));assert(Dungeon::saveExists(path));

    Dungeon restored;assert(restored.load(path));
    assert(restored.started());assert(restored.depth()==1);assert(restored.gold()==137);assert(restored.hero().heroClass()==HeroClass::Rogue);assert(restored.hero().subClass()==HeroSubClass::Assassin);assert(restored.hero().pos==heroPos);assert(restored.hero().HP==restored.hero().HT-3);assert(restored.hero().baseStrength()==12);assert(restored.hero().level()==2&&restored.hero().exp()==0);assert(restored.hero().enduranceUpgrades()==1&&restored.hero().upgradePoints()==0);assert(restored.hunger().level()==173.0f);
    assert(restored.level().cell(99)==Terrain::GRASS);assert(restored.level().plants().at(99)==SeedKind::Sungrass);assert(restored.level().blob(BlobKind::Fire)&&restored.level().blob(BlobKind::Fire)->valueAt(99)==7);assert(restored.level().visited()[77]);assert(restored.level().mapped()[88]);assert(restored.level().mobs().size()==mobs);assert(restored.level().heaps().size()==heaps);assert(restored.hero().belongings().getItem<Pasty>());PotionOfHealing* loadedPotion=restored.hero().belongings().getItem<PotionOfHealing>();assert(loadedPotion&&loadedPotion->quantity()==2&&loadedPotion->isKnown());
    Scroll remembered(ScrollKind::Terror);assert(remembered.isKnown());
    Weapon*restoredWeapon=dynamic_cast<Weapon*>(restored.hero().belongings().weapon());assert(restoredWeapon&&restoredWeapon->imbue()==Weapon::Imbue::Accuracy);SpecialItem*restoredVial=nullptr;for(Item*owned:restored.hero().belongings().allItems())if(SpecialItem*candidate=dynamic_cast<SpecialItem*>(owned))if(candidate->kind()==SpecialKind::DewVial)restoredVial=candidate;assert(restoredVial&&restoredVial->quantity()==6);
    Poison*loadedPoison=dynamic_cast<Poison*>(restored.hero().findBuff(typeid(Poison)));assert(loadedPoison&&loadedPoison->left()==13.0f);
    Combo*loadedCombo=dynamic_cast<Combo*>(restored.hero().findBuff(typeid(Combo)));assert(loadedCombo&&loadedCombo->count()==2);
    Charm*loadedCharm=dynamic_cast<Charm*>(restored.hero().findBuff(typeid(Charm)));assert(loadedCharm&&loadedCharm->object==77);
    Poison*loadedMobPoison=dynamic_cast<Poison*>(restored.level().mobs().front()->findBuff(typeid(Poison)));assert(loadedMobPoison&&loadedMobPoison->left()==9.0f);
    Goo*loadedGoo=nullptr;ZoneBoss*loadedTengu=nullptr;for(const auto&mob:restored.level().mobs()){if(Goo*g=dynamic_cast<Goo*>(mob.get()))loadedGoo=g;if(ZoneBoss*b=dynamic_cast<ZoneBoss*>(mob.get()))loadedTengu=b;}assert(loadedGoo&&loadedGoo->pumpedUp());assert(loadedTengu&&loadedTengu->sealed()&&loadedTengu->summonsSpawned()&&loadedTengu->jumpDelay()==2);

    std::fstream corrupt(path,std::ios::in|std::ios::out|std::ios::binary);assert(corrupt);char byte=0;corrupt.seekg(12);corrupt.read(&byte,1);byte^=0x55;corrupt.seekp(12);corrupt.write(&byte,1);corrupt.close();Dungeon rejected;assert(!rejected.load(path));
    std::remove(path.c_str());std::cout<<"PIXEL DUNGEON CONSOLE EDITION v0.18 save + load tests: OK\n";return 0;
}
