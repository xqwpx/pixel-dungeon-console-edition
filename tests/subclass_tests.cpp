#include <cassert>
#include <iostream>
#include <memory>
#include <vector>

#include "game/Dungeon.h"
#include "game/actors/buffs/Combo.h"
#include "game/actors/buffs/Fury.h"
#include "game/items/SpecialItems.h"
#include "game/items/ConsumableCatalog.h"
#include "game/actors/blobs/Blob.h"
#include "game/actors/buffs/Barkskin.h"
#include "game/items/potions/Potion.h"
#include "game/items/Heap.h"
#include "game/levels/Terrain.h"

using namespace pdv;

int main() {
    Combo combo;
    assert(combo.hit(10)==0);
    assert(combo.hit(10)==0);
    assert(combo.hit(10)==2);
    assert(combo.count()==3);

    Dungeon rogue;
    assert(rogue.startNew(HeroClass::Rogue,0x5A8C1234u));
    SpecialItem tome(SpecialKind::TomeOfMastery);
    const std::vector<ItemAction> paths=tome.actions(rogue.hero());
    assert(paths.size()==3);
    assert(paths[0]==ItemAction::Assassin&&paths[1]==ItemAction::Freerunner);

    Hero berserker;
    berserker.initialize(HeroClass::Warrior,0);
    berserker.setSubClass(HeroSubClass::Berserker);
    berserker.HP=berserker.HT;
    berserker.damage(static_cast<int>(berserker.HT*0.7f),nullptr);
    assert(berserker.HP>0&&berserker.hasBuff(typeid(Fury)));

    Hero freerunner;
    freerunner.initialize(HeroClass::Rogue,0);
    freerunner.setSubClass(HeroSubClass::Freerunner);
    assert(freerunner.speed()>1.5f);

    Dungeon plants;
    assert(plants.startNew(HeroClass::Huntress,0x91A271u));
    plants.hero().setSubClass(HeroSubClass::Warden);
    const int cell=plants.hero().pos;
    assert(plants.level().plant(SeedKind::Firebloom,cell));
    plants.level().press(cell,&plants.hero());
    assert(plants.level().plants().find(cell)==plants.level().plants().end());
    assert(plants.level().blob(BlobKind::Fire)&&plants.level().blob(BlobKind::Fire)->volume()>0);
    assert(plants.hero().hasBuff(typeid(Barkskin)));

    Dungeon wells;
    assert(wells.startNew(HeroClass::Warrior,0x7711AAu));
    const int well=wells.hero().pos;
    wells.hero().HP=1;
    wells.level().setRaw(well,Terrain::WELL);wells.level().buildFlagMaps();
    assert(wells.level().seedBlob(BlobKind::WaterHealth,well,1));
    wells.level().press(well,&wells.hero());
    assert(wells.hero().HP==wells.hero().HT&&wells.level().cell(well)==Terrain::EMPTY_WELL);

    int pot=well+1;
    wells.level().setRaw(pot,Terrain::ALCHEMY);wells.level().buildFlagMaps();
    assert(wells.level().seedBlob(BlobKind::Alchemy,pot,1));
    std::unique_ptr<CatalogSeed> seeds(new CatalogSeed(SeedKind::Sungrass));seeds->quantity(3);
    wells.level().drop(std::move(seeds),pot);
    assert(wells.level().heapAt(pot)&&dynamic_cast<Potion*>(wells.level().heapAt(pot)->peek()));

    std::cout<<"PIXEL DUNGEON CONSOLE EDITION subclass parity tests: OK\n";
}
