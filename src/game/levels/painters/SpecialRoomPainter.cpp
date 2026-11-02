#include "game/levels/painters/SpecialRoomPainter.h"
#include "game/levels/painters/Painter.h"
#include "game/levels/RegularLevel.h"
#include "game/levels/Room.h"
#include "game/levels/Terrain.h"
#include "game/items/Generator.h"
#include "game/items/ConsumableCatalog.h"
#include "game/items/keys/IronKey.h"
#include "game/items/keys/GoldenKey.h"
#include "game/items/potions/PotionOfLiquidFlame.h"
#include "game/items/potions/PotionOfLevitation.h"
#include "game/items/potions/PotionOfInvisibility.h"
#include "game/items/potions/Potion.h"
#include "game/items/scrolls/Scroll.h"
#include "game/items/Gold.h"
#include "game/items/Heap.h"
#include "game/items/weapon/Weapon.h"
#include "game/items/weapon/missiles/MissileWeapon.h"
#include "game/actors/blobs/Blob.h"
#include "game/actors/mobs/Piranha.h"
#include "game/actors/mobs/SpecialMob.h"
#include "game/actors/mobs/npcs/DungeonNPC.h"
#include "game/Dungeon.h"
#include "game/utils/Random.h"
#include "game/Progression.h"
#include <stdlib.h>
#include <vector>

namespace pdv {
namespace {
std::shared_ptr<Room::Door> entranceDoor(Room& room) {
    const auto doors = room.doors();
    return doors.empty() ? nullptr : doors.front();
}

int emptyCell(Level& level, const Room& room, int terrain) {
    for (int tries=0; tries<50; ++tries) {
        const Point p=room.random();
        const int cell=p.y*Level::Width+p.x;
        if(level.cell(cell)==terrain && !level.heapAt(cell)) return cell;
    }
    return -1;
}

void addKey(Level& level) {
    const RegularLevel* regular=dynamic_cast<const RegularLevel*>(&level);
    level.addItemToSpawn(std::unique_ptr<Item>(new IronKey(regular?regular->depth():1)));
}
}

void SpecialRoomPainter::garden(Level& level,Room& room) {
    Painter::fill(level,room,Terrain::WALL);
    Painter::fill(level,room,1,Terrain::HIGH_GRASS);
    Painter::fill(level,room,2,Terrain::GRASS);
    for(int y=room.top+1;y<room.bottom;++y)for(int x=room.left+1;x<room.right;++x)level.seedBlob(BlobKind::Foliage,y*Level::Width+x,1);
    if(Random::Int(2)==0){
        const Point point=room.random();
        level.drop(std::unique_ptr<Item>(new CatalogMisc(MiscKind::Honeypot)),point.y*Level::Width+point.x);
    }else{
        const int bushes=Random::Int(5)==0?2:1;
        for(int i=0;i<bushes;++i){const Point p=room.random();const int cell=p.y*Level::Width+p.x;level.setRaw(cell,Terrain::GRASS);level.plant(SeedKind::Sungrass,cell);}
    }
    if(auto door=entranceDoor(room)) door->set(Room::Door::Type::Regular);
}

void SpecialRoomPainter::library(Level& level,Room& room) {
    Painter::fill(level,room,Terrain::WALL);
    Painter::fill(level,room,1,Terrain::EMPTY);
    const auto door=entranceDoor(room);
    const RegularLevel* regular=dynamic_cast<const RegularLevel*>(&level);
    const int depth=regular?regular->depth():1;
    Point first(-1,-1),second(-1,-1);
    if(door){
        if(door->x==room.left){
            first=Point(room.left+1,door->y-1);second=Point(room.left+1,door->y+1);
            Painter::fill(level,room.right-1,room.top+1,1,room.height()-1,Terrain::BOOKSHELF);
        }else if(door->x==room.right){
            first=Point(room.right-1,door->y-1);second=Point(room.right-1,door->y+1);
            Painter::fill(level,room.left+1,room.top+1,1,room.height()-1,Terrain::BOOKSHELF);
        }else if(door->y==room.top){
            first=Point(door->x+1,room.top+1);second=Point(door->x-1,room.top+1);
            Painter::fill(level,room.left+1,room.bottom-1,room.width()-1,1,Terrain::BOOKSHELF);
        }else{
            first=Point(door->x+1,room.bottom-1);second=Point(door->x-1,room.bottom-1);
            Painter::fill(level,room.left+1,room.top+1,room.width()-1,1,Terrain::BOOKSHELF);
        }
    }
    for(const Point point:{first,second})if(level.inBounds(point.x,point.y)&&level.cell(point.y*Level::Width+point.x)==Terrain::EMPTY)Painter::set(level,point,Terrain::STATUE);
    for(int i=0;i<Random::IntRange(2,3);++i){
        const int cell=emptyCell(level,room,Terrain::EMPTY);
        std::unique_ptr<Item> item=level.itemToSpawnAsPrize();
        if(!dynamic_cast<Scroll*>(item.get())){
            if(item)level.addItemToSpawn(std::move(item));
            item=Generator::random(Generator::Category::Scroll,depth);
        }
        if(cell>=0&&item) level.drop(std::move(item),cell);
    }
    if(door) door->set(Room::Door::Type::Locked);
    addKey(level);
}

void SpecialRoomPainter::laboratory(Level& level,Room& room) {
    Painter::fill(level,room,Terrain::WALL);
    Painter::fill(level,room,1,Terrain::EMPTY_SP);
    const auto door=entranceDoor(room);
    const RegularLevel* regular=dynamic_cast<const RegularLevel*>(&level);
    const int depth=regular?regular->depth():1;
    Point pot(room.right-1,room.bottom-1);
    if(door){
        if(door->x==room.left)pot=Point(room.right-1,Random::Int(2)==0?room.top+1:room.bottom-1);
        else if(door->x==room.right)pot=Point(room.left+1,Random::Int(2)==0?room.top+1:room.bottom-1);
        else if(door->y==room.top)pot=Point(Random::Int(2)==0?room.left+1:room.right-1,room.bottom-1);
        else pot=Point(Random::Int(2)==0?room.left+1:room.right-1,room.top+1);
    }
    Painter::set(level,pot,Terrain::ALCHEMY);
    level.seedBlob(BlobKind::Alchemy,pot.y*Level::Width+pot.x,1);
    for(int i=0;i<Random::IntRange(2,3);++i){
        const int cell=emptyCell(level,room,Terrain::EMPTY_SP);
        std::unique_ptr<Item> item=level.itemToSpawnAsPrize();
        if(!dynamic_cast<Potion*>(item.get())){
            if(item)level.addItemToSpawn(std::move(item));
            item=Generator::random(Generator::Category::Potion,depth);
        }
        if(cell>=0&&item) level.drop(std::move(item),cell);
    }
    if(door) door->set(Room::Door::Type::Locked);
    addKey(level);
}

void SpecialRoomPainter::paint(Level& level,Room& room) {
    const RegularLevel*journalLevel=dynamic_cast<const RegularLevel*>(&level);const int journalDepth=journalLevel?journalLevel->depth():1;(void)journalLevel;(void)journalDepth;
    if(room.type==Room::Type::Garden){garden(level,room);return;}
    if(room.type==Room::Type::Library){library(level,room);return;}
    if(room.type==Room::Type::Laboratory){laboratory(level,room);return;}

    Painter::fill(level,room,Terrain::WALL);
    Painter::fill(level,room,1,Terrain::EMPTY);
    auto door=entranceDoor(room);

    switch(room.type){
    case Room::Type::Treasury:{
        Painter::set(level,room.center(),Terrain::STATUE);const Heap::Type heapType=Random::Int(2)==0?Heap::Type::Chest:Heap::Type::Heap;
        const int piles=Random::IntRange(2,3);for(int i=0;i<piles;++i){const int cell=emptyCell(level,room,Terrain::EMPTY);if(cell<0)continue;std::unique_ptr<Gold>gold(new Gold());const RegularLevel*regular=dynamic_cast<const RegularLevel*>(&level);gold->random(regular?regular->depth():1);Heap*heap=level.drop(std::move(gold),cell);if(heap)heap->type=i==0&&heapType==Heap::Type::Chest?Heap::Type::Mimic:heapType;}
        if(heapType==Heap::Type::Heap)for(int i=0;i<6;++i){const int cell=emptyCell(level,room,Terrain::EMPTY);if(cell>=0)level.drop(std::unique_ptr<Item>(new Gold(Random::IntRange(1,3))),cell);}if(door)door->set(Room::Door::Type::Locked);addKey(level);break;
    }
    case Room::Type::Armory:{
        if(door){Point statue=room.center();if(door->x==room.left)statue=Point(room.right-1,Random::Int(2)?room.top+1:room.bottom-1);else if(door->x==room.right)statue=Point(room.left+1,Random::Int(2)?room.top+1:room.bottom-1);else if(door->y==room.top)statue=Point(Random::Int(2)?room.left+1:room.right-1,room.bottom-1);else statue=Point(Random::Int(2)?room.left+1:room.right-1,room.top+1);Painter::set(level,statue,Terrain::STATUE);}
        const RegularLevel*regular=dynamic_cast<const RegularLevel*>(&level);const int depth=regular?regular->depth():1;const int count=3+(Random::Int(4)==0?1:0);for(int i=0;i<count;++i){const int cell=emptyCell(level,room,Terrain::EMPTY);std::unique_ptr<Item>item=Random::Int(6)==0?std::unique_ptr<Item>(new CatalogMisc(MiscKind::Bomb)):Generator::random(Random::Int(2)?Generator::Category::Weapon:Generator::Category::Armor,depth);if(cell>=0&&item)level.drop(std::move(item),cell);}
        if(door) door->set(Room::Door::Type::Locked);
        addKey(level);break;
    }
    case Room::Type::Vault:{
        Painter::fill(level,room,1,Terrain::EMPTY_SP);Painter::fill(level,room,2,Terrain::EMPTY);const Point center=room.center();const int c=center.y*Level::Width+center.x;const RegularLevel*regular=dynamic_cast<const RegularLevel*>(&level);const int depth=regular?regular->depth():1;auto prize=[&](){return Generator::random(Random::Int(2)?Generator::Category::Wand:Generator::Category::Ring,depth);};const int variant=Random::Int(3);if(variant==0){Heap*heap=level.drop(prize(),c);if(heap)heap->type=Heap::Type::LockedChest;level.addItemToSpawn(std::unique_ptr<Item>(new GoldenKey(depth)));}else if(variant==1){std::unique_ptr<Item>first=prize(),second=prize();for(int tries=0;first&&second&&first->sameKind(*second)&&tries<20;++tries)second=prize();Heap*a=level.drop(std::move(first),c);Heap*b=level.drop(std::move(second),c+Level::Neighbours8[static_cast<std::size_t>(Random::Int(8))]);if(a)a->type=Heap::Type::CrystalChest;if(b)b->type=Heap::Type::CrystalChest;level.addItemToSpawn(std::unique_ptr<Item>(new GoldenKey(depth)));}else{Painter::set(level,center,Terrain::PEDESTAL);level.drop(prize(),c);}if(door)door->set(Room::Door::Type::Locked);addKey(level);break;
    }
    case Room::Type::Storage:{
        Painter::fill(level,room,1,Terrain::EMPTY_SP);const RegularLevel*regular=dynamic_cast<const RegularLevel*>(&level);const int depth=regular?regular->depth():1;for(int i=0;i<Random::IntRange(3,4);++i){const int cell=emptyCell(level,room,Terrain::EMPTY_SP);std::unique_ptr<Item>item=level.itemToSpawnAsPrize();if(!item){static const Generator::Category prizes[]={Generator::Category::Potion,Generator::Category::Scroll,Generator::Category::Food,Generator::Category::Gold,Generator::Category::Misc};item=Generator::random(prizes[Random::Int(5)],depth);}if(cell>=0&&item)level.drop(std::move(item),cell);}if(door)door->set(Room::Door::Type::Barricade);level.addItemToSpawn(std::unique_ptr<Item>(new PotionOfLiquidFlame()));
        break;
    }
    case Room::Type::Traps:{
        const RegularLevel* regular=dynamic_cast<const RegularLevel*>(&level);
        const int depth=regular?regular->depth():1;
        static const int commonTraps[]={Terrain::TOXIC_TRAP,Terrain::TOXIC_TRAP,Terrain::TOXIC_TRAP,Terrain::PARALYTIC_TRAP,Terrain::PARALYTIC_TRAP};
        const int trapRoll=Random::Int(6);
        const int trap=trapRoll==5?((depth+1)%5==0?Terrain::SUMMONING_TRAP:Terrain::CHASM):commonTraps[trapRoll];
        const int corridor=trap==Terrain::CHASM?Terrain::CHASM:Terrain::EMPTY;
        Painter::fill(level,room,1,trap);
        int x=room.center().x,y=room.center().y;
        if(door&&door->x==room.left){x=room.right-1;for(int yy=room.top+1;yy<room.bottom;++yy)level.setRaw(x,yy,corridor);}
        else if(door&&door->x==room.right){x=room.left+1;for(int yy=room.top+1;yy<room.bottom;++yy)level.setRaw(x,yy,corridor);}
        else if(door&&door->y==room.top){y=room.bottom-1;for(int xx=room.left+1;xx<room.right;++xx)level.setRaw(xx,y,corridor);}
        else {y=room.top+1;for(int xx=room.left+1;xx<room.right;++xx)level.setRaw(xx,y,corridor);}
        const int reward=x+y*Level::Width;
        std::unique_ptr<Item> item=level.itemToSpawnAsPrize();
        if(!item){auto prize=[&](){return Generator::random(Random::Int(2)?Generator::Category::Weapon:Generator::Category::Armor,depth);};item=prize();for(int i=0;i<3;++i){std::unique_ptr<Item> candidate=prize();if(candidate&&(!item||candidate->level()>item->level()))item=std::move(candidate);}}
        if(Random::Int(3)==0){
            if(trap==Terrain::CHASM)level.setRaw(reward,Terrain::EMPTY);
            if(Heap* heap=level.drop(std::move(item),reward))heap->type=Heap::Type::Chest;
        }else{
            level.setRaw(reward,Terrain::PEDESTAL);
            level.drop(std::move(item),reward);
        }
        if(door)door->set(Room::Door::Type::Regular);
        level.addItemToSpawn(std::unique_ptr<Item>(new PotionOfLevitation()));
        break;
    }
    case Room::Type::MagicWell:{
        const Point center=room.center();Painter::set(level,center,Terrain::WELL);level.seedBlob(static_cast<BlobKind>(static_cast<int>(BlobKind::WaterHealth)+Random::Int(3)),center.y*Level::Width+center.x,1);if(door)door->set(Room::Door::Type::Regular);break;
    }
    case Room::Type::Crypt:{
        int x=room.center().x,y=room.center().y;if(door&&door->x==room.left){Painter::set(level,Point(room.right-1,room.top+1),Terrain::STATUE);Painter::set(level,Point(room.right-1,room.bottom-1),Terrain::STATUE);x=room.right-2;}else if(door&&door->x==room.right){Painter::set(level,Point(room.left+1,room.top+1),Terrain::STATUE);Painter::set(level,Point(room.left+1,room.bottom-1),Terrain::STATUE);x=room.left+2;}else if(door&&door->y==room.top){Painter::set(level,Point(room.left+1,room.bottom-1),Terrain::STATUE);Painter::set(level,Point(room.right-1,room.bottom-1),Terrain::STATUE);y=room.bottom-2;}else{Painter::set(level,Point(room.left+1,room.top+1),Terrain::STATUE);Painter::set(level,Point(room.right-1,room.top+1),Terrain::STATUE);y=room.top+2;}const RegularLevel*regular=dynamic_cast<const RegularLevel*>(&level);std::unique_ptr<Item>best=Generator::random(Generator::Category::Armor,regular?regular->depth():1);for(int i=0;i<3;++i){std::unique_ptr<Item>candidate=Generator::random(Generator::Category::Armor,regular?regular->depth():1);if(candidate&&(!best||candidate->level()>best->level()))best=std::move(candidate);}Heap*heap=level.drop(std::move(best),x+y*Level::Width);if(heap)heap->type=Heap::Type::Tomb;if(door)door->set(Room::Door::Type::Locked);addKey(level);break;
    }
    case Room::Type::Statue:{
        Point statue=room.center();
        if(door){
            door->set(Room::Door::Type::Locked);
            addKey(level);
            if(door->x==room.left){
                Painter::fill(level,room.right-1,room.top+1,1,room.height()-1,Terrain::STATUE);
                statue.x=room.right-2;
            }else if(door->x==room.right){
                Painter::fill(level,room.left+1,room.top+1,1,room.height()-1,Terrain::STATUE);
                statue.x=room.left+2;
            }else if(door->y==room.top){
                Painter::fill(level,room.left+1,room.bottom-1,room.width()-1,1,Terrain::STATUE);
                statue.y=room.bottom-2;
            }else if(door->y==room.bottom){
                Painter::fill(level,room.left+1,room.top+1,room.width()-1,1,Terrain::STATUE);
                statue.y=room.top+2;
            }
        }
        const RegularLevel*regular=dynamic_cast<const RegularLevel*>(&level);
        std::unique_ptr<SpecialMob>mob(new SpecialMob(SpecialMobKind::Statue,regular?regular->depth():1));
        mob->pos=statue.y*Level::Width+statue.x;
        level.addMob(std::move(mob));
        break;
    }
    case Room::Type::Pool:{
        Painter::fill(level,room,1,Terrain::WATER);Point reward=room.center();if(door&&door->x==room.left)reward=Point(room.right-1,room.top+room.height()/2);else if(door&&door->x==room.right)reward=Point(room.left+1,room.top+room.height()/2);else if(door&&door->y==room.top)reward=Point(room.left+room.width()/2,room.bottom-1);else if(door)reward=Point(room.left+room.width()/2,room.top+1);Painter::set(level,reward,Terrain::PEDESTAL);
        const RegularLevel* regular=dynamic_cast<const RegularLevel*>(&level);const int depth=regular?regular->depth():1;
        std::unique_ptr<Item>best=level.itemToSpawnAsPrize();if(!best){auto prize=[&](){return Generator::random(Random::Int(2)?Generator::Category::Weapon:Generator::Category::Armor,depth);};best=prize();for(int i=0;i<4;++i){std::unique_ptr<Item>candidate=prize();if(candidate&&(!best||candidate->level()>best->level()))best=std::move(candidate);}}Heap*rewardHeap=level.drop(std::move(best),reward.y*Level::Width+reward.x);if(rewardHeap&&Random::Int(3)==0)rewardHeap->type=Heap::Type::Chest;level.addItemToSpawn(std::unique_ptr<Item>(new PotionOfInvisibility()));
        for(int i=0;i<3;++i){for(int attempt=0;attempt<100;++attempt){const Point point=room.random();const int cell=point.y*Level::Width+point.x;if(level.cell(cell)!=Terrain::WATER||level.mobAt(cell))continue;std::unique_ptr<Piranha> fish(new Piranha(depth));fish->pos=cell;level.addMob(std::move(fish));break;}}
        if(door) door->set(Room::Door::Type::Regular);
        break;
    }
    case Room::Type::WeakFloor:{
        Painter::fill(level,room,1,Terrain::CHASM);if(door&&door->x==room.left)for(int y=room.top+1;y<room.bottom;++y)for(int x=room.left+1,n=Random::IntRange(1,room.width()-2);x<=room.left+n;++x)level.setRaw(x,y,Terrain::EMPTY_SP);else if(door&&door->x==room.right)for(int y=room.top+1;y<room.bottom;++y)for(int x=room.right-1,n=Random::IntRange(1,room.width()-2);x>=room.right-n;--x)level.setRaw(x,y,Terrain::EMPTY_SP);else if(door&&door->y==room.top)for(int x=room.left+1;x<room.right;++x)for(int y=room.top+1,n=Random::IntRange(1,room.height()-2);y<=room.top+n;++y)level.setRaw(x,y,Terrain::EMPTY_SP);else for(int x=room.left+1;x<room.right;++x)for(int y=room.bottom-1,n=Random::IntRange(1,room.height()-2);y>=room.bottom-n;--y)level.setRaw(x,y,Terrain::EMPTY_SP);if(door)door->set(Room::Door::Type::Regular);break;
    }
    case Room::Type::Pit:{
        Point well=room.center();if(door&&door->x==room.left)well=Point(room.right-1,Random::Int(2)?room.top+1:room.bottom-1);else if(door&&door->x==room.right)well=Point(room.left+1,Random::Int(2)?room.top+1:room.bottom-1);else if(door&&door->y==room.top)well=Point(Random::Int(2)?room.left+1:room.right-1,room.bottom-1);else if(door)well=Point(Random::Int(2)?room.left+1:room.right-1,room.top+1);Painter::set(level,well,Terrain::EMPTY_WELL);int remains=emptyCell(level,room,Terrain::EMPTY);if(remains>=0){const RegularLevel*regular=dynamic_cast<const RegularLevel*>(&level);const int depth=regular?regular->depth():1;Heap*heap=level.drop(std::unique_ptr<Item>(new IronKey(depth)),remains);if(heap)heap->type=Heap::Type::Skeleton;level.drop(Generator::random(Random::Int(5)==0?Generator::Category::Ring:(Random::Int(2)?Generator::Category::Weapon:Generator::Category::Armor),depth),remains);for(int i=0;i<Random::IntRange(1,2);++i){std::unique_ptr<Item> item=level.itemToSpawnAsPrize();if(!item){static const Generator::Category prizes[]={Generator::Category::Potion,Generator::Category::Scroll,Generator::Category::Food,Generator::Category::Gold};item=Generator::random(prizes[Random::Int(4)],depth);}level.drop(std::move(item),remains);}}if(door)door->set(Room::Door::Type::Locked);break;
    }
    case Room::Type::Altar:{
        const RegularLevel*regular=dynamic_cast<const RegularLevel*>(&level);
        const int depth=regular?regular->depth():1;
        const int floor=(depth+1)%5==0?Terrain::HIGH_GRASS:Terrain::CHASM;
        Painter::fill(level,room,1,floor);
        const Point center=room.center();
        if(door){
            if(door->x==room.left||door->x==room.right){
                Point point=Painter::drawInside(level,room,Point(door->x,door->y),std::abs(door->x-center.x)-2,Terrain::EMPTY_SP);
                while(point.y!=center.y){Painter::set(level,point,Terrain::EMPTY_SP);point.y+=point.y<center.y?1:-1;}
            }else{
                Point point=Painter::drawInside(level,room,Point(door->x,door->y),std::abs(door->y-center.y)-2,Terrain::EMPTY_SP);
                while(point.x!=center.x){Painter::set(level,point,Terrain::EMPTY_SP);point.x+=point.x<center.x?1:-1;}
            }
            door->set(Room::Door::Type::Empty);
        }
        Painter::fill(level,center.x-1,center.y-1,3,3,Terrain::EMBERS);
        Painter::set(level,center,Terrain::PEDESTAL);
        level.seedBlob(BlobKind::SacrificialFire,center.y*Level::Width+center.x,5+depth*5);
        break;
    }
    case Room::Type::RatKing:{
        Painter::fill(level,room,Terrain::WALL);
        Painter::fill(level,room,1,Terrain::EMPTY_SP);
        if(door) door->set(Room::Door::Type::Hidden);

        const int doorCell=door?door->y*Level::Width+door->x:-1;
        std::vector<int> chests;
        auto addChest=[&](int cell){
            const RegularLevel* regular=dynamic_cast<const RegularLevel*>(&level);
            const int prizeDepth=regular?regular->depth():1;
            if(cell==doorCell-1||cell==doorCell+1||cell==doorCell-Level::Width||cell==doorCell+Level::Width)return;
            std::unique_ptr<Item> prize;
            switch(Random::Int(10)){
                case 0:
                    prize=Generator::random(Generator::Category::Weapon,prizeDepth);
                    if(MissileWeapon* missile=dynamic_cast<MissileWeapon*>(prize.get())) missile->quantity(1);
                    else if(prize) prize->degrade(Random::Int(3));
                    break;
                case 1:
                    prize=Generator::random(Generator::Category::Armor,prizeDepth);
                    if(prize) prize->degrade(Random::Int(3));
                    break;
                default: prize.reset(new Gold(Random::IntRange(1,5))); break;
            }
            if(Heap* heap=level.drop(std::move(prize),cell)){heap->type=Heap::Type::Chest;chests.push_back(cell);}
        };
        for(int x=room.left+1;x<room.right;++x){addChest((room.top+1)*Level::Width+x);addChest((room.bottom-1)*Level::Width+x);}
        for(int y=room.top+2;y<room.bottom-1;++y){addChest(y*Level::Width+room.left+1);addChest(y*Level::Width+room.right-1);}
        if(!chests.empty())if(Heap* mimic=level.heapAt(chests[static_cast<std::size_t>(Random::Int(static_cast<int>(chests.size())))]))mimic->type=Heap::Type::Mimic;
        std::unique_ptr<DungeonNPC> king(new DungeonNPC(NPCKind::RatKing));
        const Point position=room.random(1);
        king->pos=position.y*Level::Width+position.x;
        level.addMob(std::move(king));
        break;
    }
    case Room::Type::Blacksmith:{
        Painter::fill(level,room,Terrain::WALL);
        Painter::fill(level,room,1,Terrain::FIRE_TRAP);
        Painter::fill(level,room,2,Terrain::EMPTY_SP);
        const RegularLevel*regular=dynamic_cast<const RegularLevel*>(&level);
        const int depth=regular?regular->depth():1;
        for(int i=0;i<2;++i){
            const int cell=emptyCell(level,room,Terrain::EMPTY_SP);
            std::unique_ptr<Item> item=Generator::random(Random::Int(2)?Generator::Category::Armor:Generator::Category::Weapon,depth);
            if(cell>=0&&item)level.drop(std::move(item),cell);
        }
        for(const auto& roomDoor:room.doors()){
            if(!roomDoor)continue;
            roomDoor->set(Room::Door::Type::Unlocked);
            Painter::drawInside(level,room,Point(roomDoor->x,roomDoor->y),1,Terrain::EMPTY);
        }
        if(Dungeon* game=level.dungeonContext())if(game->blacksmithQuestKind()==0)game->setBlacksmithQuestKind(Random::Int(2)+1);
        std::unique_ptr<DungeonNPC>npc(new DungeonNPC(NPCKind::Blacksmith));
        const Point center=room.center();
        npc->pos=center.y*Level::Width+center.x;
        for(int attempt=0;attempt<30;++attempt){
            const Point point=room.random(1);
            const int cell=point.y*Level::Width+point.x;
            if(level.passable(cell)&&!level.heapAt(cell)){npc->pos=cell;break;}
        }
        level.addMob(std::move(npc));
        break;
    }
    default: if(door)door->set(Room::Door::Type::Regular);break;
    }
}
}
