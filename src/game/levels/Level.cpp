#include "game/levels/Level.h"
#include "game/levels/Terrain.h"
#include "game/actors/mobs/Mob.h"
#include "game/actors/mobs/npcs/DungeonNPC.h"
#include "game/actors/blobs/Blob.h"
#include "game/actors/blobs/Fire.h"
#include "game/actors/blobs/ToxicGas.h"
#include "game/actors/blobs/ParalyticGas.h"
#include "game/actors/blobs/ConfusionGas.h"
#include "game/actors/blobs/WellWater.h"
#include "game/actors/blobs/Alchemy.h"
#include "game/actors/blobs/Web.h"
#include "game/actors/blobs/Regrowth.h"
#include "game/actors/blobs/Foliage.h"
#include "game/actors/blobs/SacrificialFire.h"
#include "game/Dungeon.h"
#include "game/levels/traps/ToxicTrap.h"
#include "game/levels/traps/FireTrap.h"
#include "game/levels/traps/ParalyticTrap.h"
#include "game/levels/traps/PoisonTrap.h"
#include "game/levels/traps/AlarmTrap.h"
#include "game/levels/traps/GrippingTrap.h"
#include "game/levels/traps/LightningTrap.h"
#include "game/levels/traps/SummoningTrap.h"
#include "game/actors/Hero.h"
#include "game/actors/buffs/Blindness.h"
#include "game/actors/buffs/Awareness.h"
#include "game/actors/buffs/MindVision.h"
#include "game/actors/buffs/Shadows.h"
#include "game/actors/buffs/Barkskin.h"
#include "game/actors/buffs/HerbalArmor.h"
#include "game/actors/buffs/HerbalHealing.h"
#include "game/actors/buffs/Frost.h"
#include "game/actors/buffs/Poison.h"
#include "game/actors/buffs/Roots.h"
#include "game/items/Heap.h"
#include "game/items/Item.h"
#include "game/items/ConsumableCatalog.h"
#include "game/items/SpecialItems.h"
#include "game/items/Generator.h"
#include "game/utils/Random.h"
#include "game/mechanics/ShadowCaster.h"
#include <algorithm>
#include <cstddef>
#include <cmath>
namespace pdv {

Level::Level() = default;
Level::~Level() = default;

const std::array<int,4> Level::Neighbours4={-Width,+1,+Width,-1};
const std::array<int,8> Level::Neighbours8={+1,-1,+Width,-Width,+1+Width,+1-Width,-1+Width,-1-Width};
const std::array<int,9> Level::Neighbours9={0,+1,-1,+Width,-Width,+1+Width,+1-Width,-1+Width,-1-Width};
bool Level::inBounds(int x,int y) const{return x>=0&&y>=0&&x<Width&&y<Height;}
int Level::cell(int x,int y) const{return inBounds(x,y)?map_[y*Width+x]:Terrain::WALL;}
int Level::cell(int p) const{return inBoundsCell(p)?map_[p]:Terrain::WALL;}
void Level::setRaw(int x,int y,int t){if(inBounds(x,y))setRaw(y*Width+x,t);}
void Level::setRaw(int p,int t){if(inBoundsCell(p))map_[p]=t;}
void Level::beginGeneration(Feeling f){
    clearBlobs();
    clearHeaps();
    clearMobs();
    clearPlants();
    feeling_=f; map_.fill(f==Feeling::Chasm?Terrain::CHASM:Terrain::WALL);
    visited_.fill(false);mapped_.fill(false);fieldOfView_.fill(false);entrance_=exit_=0;
}
void Level::finishGeneration(){buildFlagMaps();cleanWalls();}
int Level::tunnelTile() const{return feeling()==Feeling::Chasm?Terrain::EMPTY_SP:Terrain::EMPTY;}
void Level::createCoreParityTestMap(){
    beginGeneration(Feeling::None);
    auto room=[this](int x0,int y0,int x1,int y1,int t=Terrain::EMPTY){for(int y=y0;y<=y1;++y)for(int x=x0;x<=x1;++x)setRaw(x,y,t);};
    room(2,2,11,9);room(15,3,27,11);room(5,15,17,25);room(21,17,29,28);
    for(int x=11;x<=15;++x)setRaw(x,6,Terrain::EMPTY);
    for(int y=6;y<=18;++y)setRaw(16,y,Terrain::EMPTY);
    for(int x=10;x<=24;++x)setRaw(x,18,Terrain::EMPTY);
    setRaw(13,6,Terrain::DOOR);setRaw(8,5,Terrain::GRASS);setRaw(9,5,Terrain::HIGH_GRASS);
    setRaw(7,7,Terrain::EMBERS);setRaw(24,8,Terrain::WATER);setRaw(25,8,Terrain::WATER);
    setRaw(24,9,Terrain::WATER);setRaw(25,9,Terrain::WATER);
    entrance_=4+5*Width;exit_=27+25*Width;setRaw(entrance_,Terrain::ENTRANCE);setRaw(exit_,Terrain::EXIT);finishGeneration();
}
void Level::createDeveloperTestMap(){
    beginGeneration(Feeling::None);
    // One large, fully visible arena leaves enough spaced cells for the
    // complete bestiary and item catalogue without altering generated floors.
    for(int y=1;y<Height-1;++y)for(int x=1;x<Width-1;++x)setRaw(x,y,Terrain::EMPTY);
    entrance_=3+3*Width;
    exit_=28+28*Width;
    setRaw(entrance_,Terrain::ENTRANCE);
    setRaw(exit_,Terrain::EXIT);
    finishGeneration();
}
void Level::buildFlagMaps(){
    const auto& tf=Terrain::flags();
    for(int i=0;i<Length;++i){
        int f=tf[map_[i]];
        passable_[i]=f&Terrain::PASSABLE;losBlocking_[i]=f&Terrain::LOS_BLOCKING;
        flamable_[i]=f&Terrain::FLAMABLE;secret_[i]=f&Terrain::SECRET;solid_[i]=f&Terrain::SOLID;
        avoid_[i]=f&Terrain::AVOID;water_[i]=f&Terrain::LIQUID;pit_[i]=f&Terrain::PIT;
    }
    const int last=Length-Width;
    for(int i=0;i<Width;++i){passable_[i]=avoid_[i]=false;passable_[last+i]=avoid_[last+i]=false;}
    for(int i=Width;i<last;i+=Width){passable_[i]=avoid_[i]=false;passable_[i+Width-1]=avoid_[i+Width-1]=false;}
    for(int i=Width;i<Length-Width;++i){
        if(water_[i])map_[i]=getWaterTile(i);
        if(pit_[i]&&!pit_[i-Width]){
            int c=map_[i-Width];
            if(c==Terrain::EMPTY_SP||c==Terrain::STATUE_SP)map_[i]=Terrain::CHASM_FLOOR_SP;
            else if(water_[i-Width])map_[i]=Terrain::CHASM_WATER;
            else if((tf[c]&Terrain::UNSTITCHABLE)!=0)map_[i]=Terrain::CHASM_WALL;
            else map_[i]=Terrain::CHASM_FLOOR;
        }
    }
}
int Level::getWaterTile(int pos) const{
    int t=Terrain::WATER_TILES;const auto& tf=Terrain::flags();
    for(std::size_t j=0;j<Neighbours4.size();++j){int n=pos+Neighbours4[j];if(n>=0&&n<Length&&(tf[map_[n]]&Terrain::UNSTITCHABLE))t+=1<<static_cast<int>(j);}
    return t;
}
void Level::cleanWalls(){
    discoverable_.fill(false);
    for(int i=0;i<Length;++i){
        bool d=false;
        for(int off:Neighbours9){int n=i+off;if(n>=0&&n<Length&&map_[n]!=Terrain::WALL&&map_[n]!=Terrain::WALL_DECO){d=true;break;}}
        if(d){d=false;for(int off:Neighbours9){int n=i+off;if(n>=0&&n<Length&&!pit_[n]){d=true;break;}}}
        discoverable_[i]=d;
    }
}
void Level::updateFieldOfView(int pos,int vd){if(!inBoundsCell(pos)){fieldOfView_.fill(false);return;}ShadowCaster::castShadow(pos%Width,pos/Width,losBlocking_,fieldOfView_,vd);}
void Level::updateFieldOfViewLimited(int pos,int sense){
    fieldOfView_.fill(false);
    if(!inBoundsCell(pos)) return;

    const int cx=pos%Width;
    const int cy=pos/Width;

    for(int y=cy-sense;y<=cy+sense;++y){
        for(int x=cx-sense;x<=cx+sense;++x){
            if(inBounds(x,y)){
                const int p=y*Width+x;
                fieldOfView_[static_cast<std::size_t>(p)] =
                    discoverable_[static_cast<std::size_t>(p)];
            }
        }
    }
}

int Level::distance(int a,int b){return std::max(std::abs(a%Width-b%Width),std::abs(a/Width-b/Width));}
bool Level::adjacent(int a,int b){int d=std::abs(a-b);return d==1||d==Width||d==Width+1||d==Width-1;}

int Level::randomDestination() {
    for (int guard = 0; guard < Length * 4; ++guard) {
        const int cell = Random::Int(Length);

        if (passable(cell)) {
            return cell;
        }
    }

    return -1;
}

Mob* Level::mobAt(int pos) const {
    for (const auto& mob : mobs_) {
        if (mob &&
            mob->isAlive() &&
            mob->pos == pos) {
            return mob.get();
        }
    }

    return nullptr;
}

void Level::clearMobs() {
    mobs_.clear();
}

void Level::addMob(std::unique_ptr<Mob> mob) {
    if (mob) {
        mobs_.push_back(
            std::move(mob)
        );
    }
}


int Level::purgeDeadMobs() {
    const std::size_t before = mobs_.size();

    mobs_.erase(
        std::remove_if(
            mobs_.begin(),
            mobs_.end(),
            [](const std::unique_ptr<Mob>& mob) {
                return !mob || !mob->isAlive();
            }
        ),
        mobs_.end()
    );

    return static_cast<int>(before - mobs_.size());
}

Heap* Level::heapAt(int pos) const {
    const auto it = heaps_.find(pos);
    return it == heaps_.end() ? nullptr : it->second.get();
}

Heap* Level::drop(std::unique_ptr<Item> item, int pos) {
    if (!item || !inBoundsCell(pos)) return nullptr;
    // Door cells are a navigation choke point.  A dropped item there can hide
    // the door and, for closed doors, become unreachable.  Move it to an
    // adjacent free floor instead.
    const int terrain = cell(pos);
    if (terrain == Terrain::DOOR || terrain == Terrain::OPEN_DOOR ||
        terrain == Terrain::LOCKED_DOOR || terrain == Terrain::SECRET_DOOR) {
        std::vector<int> destinations;
        for (const int offset : Neighbours8) {
            const int destination = pos + offset;
            if (!inBoundsCell(destination) || heapAt(destination) || mobAt(destination)) continue;
            const int destinationTerrain = cell(destination);
            if (passable(destination) && destinationTerrain != Terrain::DOOR &&
                destinationTerrain != Terrain::OPEN_DOOR &&
                destinationTerrain != Terrain::LOCKED_DOOR &&
                destinationTerrain != Terrain::SECRET_DOOR) {
                destinations.push_back(destination);
            }
        }
        if (destinations.empty()) return nullptr;
        return drop(std::move(item), destinations[static_cast<std::size_t>(Random::Int(static_cast<int>(destinations.size())))]);
    }
    Heap* heap = heapAt(pos);
    if (heap && (heap->type == Heap::Type::LockedChest || heap->type == Heap::Type::CrystalChest)) {
        std::vector<int> destinations;
        for (const int offset : Neighbours8) {
            const int destination = pos + offset;
            if (inBoundsCell(destination) && (passable(destination) || avoid(destination))) destinations.push_back(destination);
        }
        if (destinations.empty()) return nullptr;
        return drop(std::move(item), destinations[static_cast<std::size_t>(Random::Int(static_cast<int>(destinations.size())))]);
    }
    if (!heap) {
        std::unique_ptr<Heap> created(new Heap(pos));
        heap = created.get();
        heaps_[pos] = std::move(created);
    }
    heap->drop(std::move(item));
    if(dungeon_&&(cell(pos)==Terrain::WELL||cell(pos)==Terrain::ALCHEMY)){press(pos,nullptr);return heapAt(pos);}
    return heap;
}

void Level::clearHeaps() {
    heaps_.clear();
}


void Level::removeEmptyHeap(int pos) {
    auto it=heaps_.find(pos);
    if (it!=heaps_.end() && (!it->second || it->second->empty())) heaps_.erase(it);
}



void Level::updateFieldOfViewFor(Char& ch) {
    const int cx = ch.pos % Width;
    const int cy = ch.pos / Width;

    const bool sighted =
        ch.isAlive() &&
        !ch.hasBuff(typeid(Blindness)) &&
        !ch.hasBuff(typeid(Shadows));

    if (sighted) {
        ShadowCaster::castShadow(
            cx,
            cy,
            losBlocking_,
            fieldOfView_,
            ch.viewDistance
        );
    } else {
        fieldOfView_.fill(false);
    }

    int sense = 1;

    if (ch.isAlive()) {
        if (const MindVision* mind =
                dynamic_cast<const MindVision*>(
                    ch.findBuff(typeid(MindVision))
                )) {
            sense =
                std::max(
                    mind->distance,
                    sense
                );
        }
    }

    if ((sighted && sense > 1) ||
        !sighted) {

        const int ax =
            std::max(
                0,
                cx - sense
            );

        const int bx =
            std::min(
                cx + sense,
                Width - 1
            );

        const int ay =
            std::max(
                0,
                cy - sense
            );

        const int by =
            std::min(
                cy + sense,
                Height - 1
            );

        for (int y = ay; y <= by; ++y) {
            for (int x = ax; x <= bx; ++x) {
                const int cell =
                    y * Width + x;

                fieldOfView_[
                    static_cast<std::size_t>(
                        cell
                    )
                ] =
                    discoverable_[
                        static_cast<std::size_t>(
                            cell
                        )
                    ];
            }
        }
    }

    auto revealAround = [this](int center) {
        if (!inBoundsCell(center)) {
            return;
        }

        const int cx =
            center % Width;

        const int cy =
            center / Width;

        for (int y = cy - 1; y <= cy + 1; ++y) {
            for (int x = cx - 1; x <= cx + 1; ++x) {
                if (!inBounds(x, y)) {
                    continue;
                }

                const int cell =
                    y * Width + x;

                fieldOfView_[
                    static_cast<std::size_t>(
                        cell
                    )
                ] = true;
            }
        }
    };

    if (ch.isAlive()) {
        if (ch.hasBuff(typeid(Awareness))) {
            for (const auto& entry : heaps_) {
                if (entry.second) {
                    revealAround(entry.first);
                }
            }
        }

        if (ch.hasBuff(typeid(MindVision))) {
            for (const auto& mob : mobs_) {
                if (mob && mob->isAlive()) {
                    revealAround(mob->pos);
                }
            }
        } else {
            Hero* hero =
                dynamic_cast<Hero*>(&ch);

            if (hero &&
                hero->heroClass() ==
                    HeroClass::Huntress) {

                for (const auto& mob : mobs_) {
                    if (!mob ||
                        !mob->isAlive()) {
                        continue;
                    }

                    if (distance(
                            ch.pos,
                            mob->pos
                        ) == 2) {
                        revealAround(mob->pos);
                    }
                }
            }
        }
    }
}


Blob* Level::blob(BlobKind kind) const {
    for (const auto& blob : blobs_) {
        if (blob &&
            blob->kind() == kind) {
            return blob.get();
        }
    }

    return nullptr;
}

Blob* Level::seedBlob(
    BlobKind kind,
    int cell,
    int amount
) {
    if (!inBoundsCell(cell) ||
        amount <= 0) {
        return nullptr;
    }

    Blob* result =
        blob(kind);

    if (!result) {
        std::unique_ptr<Blob> created;

        switch (kind) {
            case BlobKind::Fire:
                created.reset(new Fire());
                break;

            case BlobKind::ToxicGas:
                created.reset(new ToxicGas());
                break;

            case BlobKind::ParalyticGas:
                created.reset(new ParalyticGas());
                break;

            case BlobKind::ConfusionGas:
                created.reset(new ConfusionGas());
                break;
            case BlobKind::WaterHealth:case BlobKind::WaterAwareness:case BlobKind::WaterTransmutation:
                created.reset(new WellWater(kind));
                break;
            case BlobKind::Alchemy:
                created.reset(new Alchemy());
                break;
            case BlobKind::Web:
                created.reset(new Web());
                break;
            case BlobKind::Regrowth:
                created.reset(new Regrowth());
                break;
            case BlobKind::Foliage:
                created.reset(new Foliage());
                break;
            case BlobKind::SacrificialFire:
                created.reset(new SacrificialFire());
                break;
        }

        if (!created) {
            return nullptr;
        }

        created->bind(dungeon_);

        result = created.get();
        blobs_.push_back(
            std::move(created)
        );

        if (dungeon_) {
            dungeon_
                ->scheduler()
                .add(result);
        }
    }

    result->seed(
        cell,
        amount
    );

    return result;
}

void Level::clearBlobs() {
    // A generation retry can create room effects (for example an alchemy
    // pot) before the layout is rejected.  Blobs are Actors and register
    // themselves with the Dungeon scheduler in seedBlob().  Remove them
    // before destroying their ownership objects, otherwise the next floor
    // turn dereferences a stale Actor pointer.
    if (dungeon_) {
        for (const auto& blob : blobs_) {
            if (blob) dungeon_->scheduler().remove(blob.get());
        }
    }
    blobs_.clear();
}

bool Level::plant(SeedKind kind,int pos){
    if(!inBoundsCell(pos)||pit_[static_cast<std::size_t>(pos)]||cell(pos)==Terrain::ALCHEMY)return false;
    plants_[pos]=kind;
    return true;
}

bool Level::activatePlant(int pos,Char* ch){
    auto found=plants_.find(pos);
    if(found==plants_.end()||!dungeon_)return false;
    const SeedKind kind=found->second;
    plants_.erase(found);
    if(Hero* hero=dynamic_cast<Hero*>(ch)){
        if(hero->subClass()==HeroSubClass::Warden){
            if(Barkskin* bark=Buff::affect<Barkskin>(*hero))bark->level(hero->HT/3);
        }
    }
    switch(kind){
        case SeedKind::Firebloom:seedBlob(BlobKind::Fire,pos,2);break;
        case SeedKind::Icecap:
            for(int off:Neighbours9){const int p=pos+off;if(!inBoundsCell(p))continue;if(Blob* fire=blob(BlobKind::Fire))fire->clear(p);if(Char* target=dungeon_->scheduler().findChar(p))Buff::prolong<Frost>(*target,Frost::duration(*target)*Random::Float(1.0f,1.5f));if(Heap* heap=heapAt(p)){if(heap->type==Heap::Type::Mimic)dungeon_->activateMimicAt(p,false);else heap->freeze();}}
            break;
        case SeedKind::Sorrowmoss:if(ch){if(Poison* poison=Buff::affect<Poison>(*ch))poison->set(4+dungeon_->depth()/2);}break;
        case SeedKind::Dreamweed:seedBlob(BlobKind::ConfusionGas,pos,400);break;
        case SeedKind::Sungrass:if(ch)Buff::affect<HerbalHealing>(*ch);break;
        case SeedKind::Earthroot:if(ch){if(HerbalArmor* armor=Buff::affect<HerbalArmor>(*ch))armor->level(ch->HT);}break;
        case SeedKind::Fadeleaf:if(ch){const int target=randomDestination();if(target>=0)dungeon_->relocateChar(*ch,target);}break;
        case SeedKind::Rotberry:seedBlob(BlobKind::ToxicGas,pos,100);drop(std::unique_ptr<Item>(new CatalogSeed(SeedKind::Rotberry)),pos);if(ch)Buff::prolong<Roots>(*ch,3.0f);break;
    }
    if(dungeon_->hero().subClass()==HeroSubClass::Warden){
        if(Random::Int(5)==0)drop(Generator::random(Generator::Category::Seed,dungeon_->depth()),pos);
        if(Random::Int(5)==0)drop(std::unique_ptr<Item>(new SpecialItem(SpecialKind::Dewdrop)),pos);
    }
    return true;
}

void Level::destroy(int pos) {
    if (!inBoundsCell(pos)) {
        return;
    }

    const int terrain =
        map_[
            static_cast<std::size_t>(
                pos
            )
        ];

    if ((Terrain::flagsFor(terrain) &
         Terrain::UNSTITCHABLE) == 0) {

        setRaw(
            pos,
            Terrain::EMBERS
        );

    } else {
        bool flood = false;

        for (int offset :
             Neighbours4) {

            const int next =
                pos + offset;

            if (inBoundsCell(next) &&
                water(next)) {
                flood = true;
                break;
            }
        }

        if (flood) {
            setRaw(
                pos,
                getWaterTile(pos)
            );
        } else {
            setRaw(
                pos,
                tunnelTile()
            );
        }
    }
}

void Level::press(
    int cell,
    Char* ch
) {
    if (!inBoundsCell(cell) ||
        !dungeon_) {
        return;
    }

    if (map_[static_cast<std::size_t>(cell)] == Terrain::HIGH_GRASS) {
        setRaw(cell, Terrain::GRASS);
        buildFlagMaps();
        cleanWalls();
        int herbalism=0;if(Hero*hero=dynamic_cast<Hero*>(ch))for(const Ring*ring:{hero->belongings().ring1(),hero->belongings().ring2()})if(ring)herbalism+=ring->herbalismBonus();
        if(herbalism>=0&&Random::Int(18)<=Random::Int(herbalism+1))drop(Generator::random(Generator::Category::Seed,dungeon_->depth()),cell);
        if(herbalism>=0&&Random::Int(6)<=Random::Int(herbalism+1))drop(std::unique_ptr<Item>(new SpecialItem(SpecialKind::Dewdrop)),cell);
        if(Hero* hero=dynamic_cast<Hero*>(ch)){
            if(hero->subClass()==HeroSubClass::Warden){if(Barkskin* bark=Buff::affect<Barkskin>(*hero))bark->level(hero->HT/3);}
        }
    }

    if(map_[static_cast<std::size_t>(cell)]==Terrain::WELL){
        for(const auto& field:blobs_)if(WellWater* water=dynamic_cast<WellWater*>(field.get()))if(water->affect(cell))break;
    }else if(map_[static_cast<std::size_t>(cell)]==Terrain::ALCHEMY&&ch==nullptr){
        if(Alchemy* alchemy=dynamic_cast<Alchemy*>(blob(BlobKind::Alchemy)))alchemy->transmute(cell);
    }

    bool trap = false;

    switch (map_[
        static_cast<std::size_t>(
            cell
        )
    ]) {

        case Terrain::SECRET_TOXIC_TRAP:
        case Terrain::TOXIC_TRAP:
            trap = true;
            ToxicTrap::trigger(
                *dungeon_,
                cell,
                ch
            );
            break;

        case Terrain::SECRET_FIRE_TRAP:
        case Terrain::FIRE_TRAP:
            trap = true;
            FireTrap::trigger(
                *dungeon_,
                cell,
                ch
            );
            break;

        case Terrain::SECRET_PARALYTIC_TRAP:
        case Terrain::PARALYTIC_TRAP:
            trap = true;
            ParalyticTrap::trigger(
                *dungeon_,
                cell,
                ch
            );
            break;

        case Terrain::SECRET_POISON_TRAP:
        case Terrain::POISON_TRAP:
            trap = true;
            PoisonTrap::trigger(
                *dungeon_,
                cell,
                ch
            );
            break;

        case Terrain::SECRET_ALARM_TRAP:
        case Terrain::ALARM_TRAP:
            trap = true;
            AlarmTrap::trigger(
                *dungeon_,
                cell,
                ch
            );
            break;

        case Terrain::SECRET_GRIPPING_TRAP:
        case Terrain::GRIPPING_TRAP:
            trap = true;
            GrippingTrap::trigger(*dungeon_, cell, ch);
            break;

        case Terrain::SECRET_LIGHTNING_TRAP:
        case Terrain::LIGHTNING_TRAP:
            trap = true;
            LightningTrap::trigger(*dungeon_, cell, ch);
            break;

        case Terrain::SECRET_SUMMONING_TRAP:
        case Terrain::SUMMONING_TRAP:
            trap = true;
            SummoningTrap::trigger(*dungeon_, cell, ch);
            break;

        default:
            break;
    }

    if (trap) {
        setRaw(
            cell,
            Terrain::INACTIVE_TRAP
        );

        buildFlagMaps();
        cleanWalls();
    }

    activatePlant(cell,ch);
}

void Level::mobPress(Mob& mob) {
    if (mob.flying ||
        !inBoundsCell(mob.pos) ||
        !dungeon_) {
        return;
    }

    if(map_[static_cast<std::size_t>(mob.pos)]==Terrain::HIGH_GRASS){
        setRaw(mob.pos,Terrain::GRASS);buildFlagMaps();cleanWalls();
        if(Random::Int(18)==0)drop(Generator::random(Generator::Category::Seed,dungeon_->depth()),mob.pos);
        if(Random::Int(6)==0)drop(std::unique_ptr<Item>(new SpecialItem(SpecialKind::Dewdrop)),mob.pos);
    }

    bool trap = true;

    switch (map_[
        static_cast<std::size_t>(
            mob.pos
        )
    ]) {

        case Terrain::TOXIC_TRAP:
            ToxicTrap::trigger(
                *dungeon_,
                mob.pos,
                &mob
            );
            break;

        case Terrain::FIRE_TRAP:
            FireTrap::trigger(
                *dungeon_,
                mob.pos,
                &mob
            );
            break;

        case Terrain::PARALYTIC_TRAP:
            ParalyticTrap::trigger(
                *dungeon_,
                mob.pos,
                &mob
            );
            break;

        case Terrain::POISON_TRAP:
            PoisonTrap::trigger(
                *dungeon_,
                mob.pos,
                &mob
            );
            break;

        case Terrain::ALARM_TRAP:
            AlarmTrap::trigger(
                *dungeon_,
                mob.pos,
                &mob
            );
            break;

        case Terrain::GRIPPING_TRAP:
            GrippingTrap::trigger(*dungeon_, mob.pos, &mob);
            break;

        case Terrain::LIGHTNING_TRAP:
            LightningTrap::trigger(*dungeon_, mob.pos, &mob);
            break;

        case Terrain::SUMMONING_TRAP:
            SummoningTrap::trigger(*dungeon_, mob.pos, &mob);
            break;

        default:
            trap = false;
            break;
    }

    if (trap) {
        setRaw(
            mob.pos,
            Terrain::INACTIVE_TRAP
        );

        buildFlagMaps();
        cleanWalls();
    }

    activatePlant(mob.pos,&mob);
}


void Level::addItemToSpawn(std::unique_ptr<Item> item) {
    if (item) itemsToSpawn_.push_back(std::move(item));
}

std::unique_ptr<Item> Level::itemToSpawnAsPrize() {
    if (Random::Int(static_cast<int>(itemsToSpawn_.size()) + 1) == 0) return nullptr;
    const std::size_t index=static_cast<std::size_t>(Random::Int(static_cast<int>(itemsToSpawn_.size())));
    std::unique_ptr<Item> item=std::move(itemsToSpawn_[index]);
    itemsToSpawn_.erase(itemsToSpawn_.begin()+static_cast<std::ptrdiff_t>(index));
    return item;
}

void Level::clearItemsToSpawn() {
    itemsToSpawn_.clear();
}

}
