#include <cassert>
#include <iostream>
#include <memory>
#include "core/GameSession.h"
#include "game/Dungeon.h"
#include "game/items/Heap.h"
#include "game/actors/Hero.h"
#include "game/actors/mobs/ZoneMob.h"
#include "game/actors/buffs/Hunger.h"
#include "game/actors/buffs/Cripple.h"
#include "game/items/food/Food.h"
#include "game/items/SpecialItems.h"
#include "game/items/bags/Keyring.h"
#include "game/items/keys/IronKey.h"
#include "game/items/keys/GoldenKey.h"
#include "game/items/keys/SkeletonKey.h"
#include "game/levels/Terrain.h"

using namespace pdv;

static void startingCommonItems(){
    Hero hero; Hunger hunger; hunger.bind(&hero); hero.bindHunger(&hunger);
    hero.initialize(HeroClass::Warrior, 100);
    Food* food=hero.belongings().getItem<Food>();
    Keyring* ring=hero.belongings().getItem<Keyring>();
    assert(food && food->quantity()==1);
    assert(ring && ring->size()==12 && ring->count()==0);
}

static void keyringRouting(){
    Hero hero; Hunger hunger; hunger.bind(&hero); hero.bindHunger(&hunger);
    hero.initialize(HeroClass::Mage, 100);
    Bag& pack=hero.belongings().backpack();
    const int before=pack.count();
    assert(pack.collect(std::unique_ptr<Item>(new IronKey(3))));
    assert(pack.collect(std::unique_ptr<Item>(new GoldenKey(3))));
    assert(pack.count()==before); // both routed into Keyring
    Keyring* ring=hero.belongings().getItem<Keyring>();
    assert(ring && ring->count()==2);
    IronKey* iron=hero.belongings().getKey<IronKey>();
    assert(iron);
    auto detached=hero.belongings().detach(iron);
    assert(detached && dynamic_cast<IronKey*>(detached.get()));
    assert(ring->count()==1);
}

static void foodAndHunger(){
    Hero warrior; Hunger hunger; hunger.bind(&warrior); warrior.bindHunger(&hunger);
    warrior.initialize(HeroClass::Warrior, 100);
    warrior.HP=10;
    hunger.satisfy(-Hunger::Starving);
    assert(hunger.isStarving());
    assert(warrior.eatFood());
    assert(warrior.HP==15);
    assert(hunger.level()==100.0f); // 360 - 260
    assert(warrior.belongings().getItem<Food>()==nullptr);

    Hero rogue; Hunger rh; rh.bind(&rogue); rogue.bindHunger(&rh); rogue.initialize(HeroClass::Rogue,100);
    ActorScheduler sched; sched.add(&rh); // act directly via scheduler time model not needed
    rh.satisfy(-260); assert(rh.isHungry());
}

static void repeatedNewGameResetsSurvival(){
    Dungeon d; assert(d.startNew(HeroClass::Rogue, 0x2020u));
    d.hunger().satisfy(-300);
    assert(d.hunger().level()==300.0f);
    assert(d.startNew(HeroClass::Rogue, 0x3030u));
    assert(d.hunger().level()==0.0f);
    assert(d.hero().cooldown() <= 0.0001f);
}

static void dungeonPickupAndUnlock(){
    Dungeon d; assert(d.startNew(HeroClass::Warrior, 0x1010u));
    const int p=d.hero().pos;
    d.level().drop(std::unique_ptr<Item>(new Food()), p);
    Food* before=d.hero().belongings().getItem<Food>(); assert(before && before->quantity()==1);
    assert(d.pickUpAtHero());
    Food* after=d.hero().belongings().getItem<Food>(); assert(after && after->quantity()==2);
    assert(d.level().heapAt(p)==nullptr);

    d.hero().HP=std::max(1,d.hero().HT-3);
    const int hpBeforeDew=d.hero().HP;
    assert(d.level().drop(std::unique_ptr<Item>(new SpecialItem(SpecialKind::Dewdrop)),p));
    assert(d.pickUpAtHero());
    assert(d.hero().HP==hpBeforeDew+1);
    assert(d.level().heapAt(p)==nullptr);

    Heap* chest=d.level().drop(std::unique_ptr<Item>(new Food()),p);assert(chest);chest->type=Heap::Type::Chest;
    assert(d.pickUpAtHero());assert(d.level().heapAt(p)&&d.level().heapAt(p)->type==Heap::Type::Heap);
    assert(d.pickUpAtHero());assert(d.level().heapAt(p)==nullptr);

    Heap* hiddenChest=d.level().drop(std::unique_ptr<Item>(new SpecialItem(SpecialKind::ChestPlaceholder)),p);assert(hiddenChest);hiddenChest->type=Heap::Type::Chest;
    assert(dynamic_cast<SpecialItem*>(hiddenChest->peek())->kind()==SpecialKind::ChestPlaceholder);
    assert(d.pickUpAtHero());hiddenChest=d.level().heapAt(p);assert(hiddenChest&&hiddenChest->type==Heap::Type::Heap);
    const SpecialItem* revealed=dynamic_cast<const SpecialItem*>(hiddenChest->peek());assert(!revealed||revealed->kind()!=SpecialKind::ChestPlaceholder);

    // Manual locked-door fixture adjacent to hero.
    int target=-1;
    for(int ofs:Level::Neighbours4){ int c=p+ofs; if(d.level().inBoundsCell(c)){target=c;break;} }
    assert(target>=0);
    d.level().setRaw(target, Terrain::LOCKED_DOOR); d.level().buildFlagMaps();
    // A key acquired on a different floor must still unlock this door.
    assert(d.hero().belongings().backpack().collect(std::unique_ptr<Item>(new IronKey(d.depth()+1))));
    assert(d.hero().belongings().getKey<IronKey>());
    assert(d.unlockAt(target));
    assert(d.level().cell(target)==Terrain::DOOR);
    assert(d.hero().belongings().getKey<IronKey>()==nullptr);

    Heap* lockedChest=d.level().drop(std::unique_ptr<Item>(new Food()),p);assert(lockedChest);lockedChest->type=Heap::Type::LockedChest;
    assert(d.hero().belongings().backpack().collect(std::unique_ptr<Item>(new GoldenKey(d.depth()+1))));
    assert(d.pickUpAtHero());
    assert(lockedChest->type==Heap::Type::Heap);
    assert(d.hero().belongings().getKey<GoldenKey>()==nullptr);

    const int heroPos=d.hero().pos;int pit=-1;
    for(int ofs:Level::Neighbours4){int c=heroPos+ofs;if(d.level().inBoundsCell(c)&&d.scheduler().findChar(c)==nullptr){pit=c;break;}}
    assert(pit>=0);
    d.level().setRaw(pit,Terrain::CHASM);d.level().buildFlagMaps();
    assert(d.moveHeroBy(pit%Level::Width-heroPos%Level::Width,pit/Level::Width-heroPos/Level::Width));assert(d.depth()==2);assert(d.hero().hasBuff(typeid(Cripple)));
}

static void tenthFloorActivation(){
    Dungeon d; assert(d.startNew(HeroClass::Warrior,0x1011u));
    for(int depth=2;depth<=10;++depth){d.hero().pos=d.level().exit();assert(d.descend());assert(d.depth()==depth);}
    d.scheduler().freeCell(d.hero().pos);
    d.hero().pos=23*Level::Width+Level::Width/2;
    d.scheduler().occupyCell(&d.hero());
    d.level().press(d.hero().pos,&d.hero());
    bool tengu=false;
    for(const auto&mob:d.level().mobs())if(const ZoneBoss*boss=dynamic_cast<const ZoneBoss*>(mob.get()))if(boss->kind()==ZoneBossKind::Tengu)tengu=true;
    assert(tengu);
}

int main(){
    startingCommonItems(); keyringRouting(); foodAndHunger(); repeatedNewGameResetsSurvival(); dungeonPickupAndUnlock(); tenthFloorActivation();
    std::cout << "PIXEL DUNGEON CONSOLE EDITION v0.10 survival + inventory tests: OK\n";
    return 0;
}
