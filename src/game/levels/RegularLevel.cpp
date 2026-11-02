#include "game/levels/RegularLevel.h"
#include "game/levels/Terrain.h"
#include "game/levels/Patch.h"
#include "game/levels/painters/EntrancePainter.h"
#include "game/levels/painters/ExitPainter.h"
#include "game/levels/painters/BossExitPainter.h"
#include "game/levels/painters/SpecialRoomPainter.h"
#include "game/levels/painters/StandardPainter.h"
#include "game/levels/painters/TunnelPainter.h"
#include "game/levels/painters/PassagePainter.h"
#include "game/levels/painters/Painter.h"
#include "game/utils/Graph.h"
#include "game/utils/Random.h"
#include "game/actors/mobs/Bestiary.h"
#include "game/actors/mobs/Mob.h"
#include "game/actors/mobs/ZoneMob.h"
#include "game/actors/mobs/npcs/DungeonNPC.h"
#include "game/Dungeon.h"
#include "game/items/Generator.h"
#include "game/items/EquipmentCatalog.h"
#include "game/items/Heap.h"
#include "game/items/Gold.h"
#include "game/items/keys/IronKey.h"
#include "game/items/Item.h"
#include "game/items/SpecialItems.h"
#include "game/items/bags/SeedPouch.h"
#include "game/items/bags/ScrollHolder.h"
#include "game/items/potions/PotionOfStrength.h"
#include "game/items/potions/PotionOfHealing.h"
#include "game/items/scrolls/Scroll.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <queue>
namespace pdv {
static bool bossLevel(int depth){return depth%5==0;}

static bool validFloorLayout(const Level& level) {
 const int entrance=level.entrance(), exit=level.exit();
 if(!level.inBoundsCell(entrance)||!level.inBoundsCell(exit)||entrance==exit)return false;
 if(level.cell(entrance)!=Terrain::ENTRANCE||level.cell(exit)!=Terrain::EXIT)return false;

 std::array<bool,Level::Length> reached{};
 std::queue<int> pending;
 reached[static_cast<std::size_t>(entrance)]=true;
 pending.push(entrance);

 while(!pending.empty()){
  const int cell=pending.front();pending.pop();
  if(cell==exit)return true;
  const int x=cell%Level::Width;
  const int y=cell/Level::Width;
  for(int offset:Level::Neighbours4){
   const int next=cell+offset;
   if(!level.inBoundsCell(next)||reached[static_cast<std::size_t>(next)])continue;
   const int nx=next%Level::Width;
   const int ny=next/Level::Width;
   if(std::abs(nx-x)+std::abs(ny-y)!=1)continue;
   if(!(level.passable(next)||level.avoid(next)))continue;
   reached[static_cast<std::size_t>(next)]=true;
   pending.push(next);
  }
 }
 return false;
}

bool RegularLevel::create(int depth,uint32_t seed){
 pitRoomNeeded_=depth>1&&weakFloorCreated_;weakFloorCreated_=false;bossArenaActivated_=false;depth_=depth;seed_=seed;sealedEntrance_=0;bossEntranceCell_=0;bossArenaDoor_=0;Random::seed(seed);
 clearItemsToSpawn();
 Feeling feeling=Feeling::None;
 Dungeon* game=dungeonContext();

#if defined(PD_PLATFORM_PS1)
 // Keep PS1 generation bounded while its graph-container adapter is reduced.
 // Gameplay still uses the normal hero, mobs, loot and combat systems.
 return buildFallbackLevel();
#endif

 if(depth_==26){beginGeneration(feeling);rooms_.clear();roomEntrance_=roomExit_=nullptr;secretDoors_=0;const int size=7;for(int y=1;y<=size;++y)for(int x=1;x<=size;++x)setRaw(x,y,Terrain::WATER);for(int y=2;y<size;++y)for(int x=2;x<size;++x)setRaw(x,y,Terrain::EMPTY);for(int y=size/2;y<size/2+3;++y)for(int x=size/2;x<size/2+3;++x)setRaw(x,y,Terrain::EMPTY_SP);setEntrance(size*Width+size/2+1);setExit(entrance()-Width*size);setRaw(entrance(),Terrain::ENTRANCE);setRaw(exit(),Terrain::LOCKED_EXIT);const int pedestal=(size/2+1)*(Width+1);setRaw(pedestal,Terrain::PEDESTAL);setRaw(pedestal-1,Terrain::STATUE_SP);setRaw(pedestal+1,Terrain::STATUE_SP);finishGeneration();drop(std::unique_ptr<Item>(new SpecialItem(SpecialKind::Amulet)),pedestal);return true;}

 if(bossLevel(depth_)){
  beginGeneration(feeling);rooms_.clear();roomEntrance_=roomExit_=nullptr;secretDoors_=0;
  if(!buildBossArena())return false;
  bossEntranceCell_=entrance();
  finishGeneration();
  createMobs();
  if(game){std::unique_ptr<Item>bones=game->takeBones(depth_);if(bones){Heap*heap=drop(std::move(bones),randomRespawnCell());if(heap)heap->type=Heap::Type::Skeleton;}}
  setRaw(exit(),Terrain::LOCKED_EXIT);buildFlagMaps();
  return true;
 }

 {
  addItemToSpawn(Generator::random(Generator::Category::Food,depth_));
  if(game&&game->posNeeded()){
   addItemToSpawn(std::unique_ptr<Item>(new PotionOfStrength()));
   game->notePotionOfStrength();
  }
  if(game&&game->souNeeded()){
   addItemToSpawn(std::unique_ptr<Item>(new ScrollOfUpgrade()));
   game->noteScrollOfUpgrade();
  }
  if(game&&game->soeNeeded()){
   addItemToSpawn(std::unique_ptr<Item>(new ScrollOfEnchantment()));
   game->noteScrollOfEnchantment();
  }

  if(depth_>1){
   switch(Random::Int(10)){
    case 0: if(!bossLevel(depth_+1)) feeling=Feeling::Chasm; break;
    case 1: feeling=Feeling::Water; break;
    case 2: feeling=Feeling::Grass; break;
   }
  }
 }

 const bool needPit=pitRoomNeeded_;
 // Retrying a bad layout hundreds of times creates a visible multi-frame
 // stall on Vita.  Keep variety for normal levels, then fall back to a
 // guaranteed connected layout instead of failing a descent.
 for(int attempt=0;attempt<48;++attempt){
  pitRoomNeeded_=needPit;weakFloorCreated_=false;
  beginGeneration(feeling);rooms_.clear();roomEntrance_=roomExit_=nullptr;secretDoors_=0;
  if(build()){
   decorate();finishGeneration();
   if(!validFloorLayout(*this))continue;
   createMobs();createItems();
   if(bossLevel(depth_)){
    setRaw(exit(),Terrain::LOCKED_EXIT);
    buildFlagMaps();
   }
   return true;
  }
 }
 return buildFallbackLevel();
}
bool RegularLevel::buildFallbackLevel(){
 beginGeneration(Feeling::None);
 rooms_.clear();roomEntrance_=roomExit_=nullptr;secretDoors_=0;

 // Four simple rooms with a cross-shaped corridor.  This path is used only
 // when procedural generation could not produce a valid map in time.
 Painter::fill(*this,2,2,10,8,Terrain::EMPTY);
 Painter::fill(*this,20,3,10,8,Terrain::EMPTY);
 Painter::fill(*this,3,21,10,8,Terrain::EMPTY);
 Painter::fill(*this,20,20,10,9,Terrain::EMPTY);
 Painter::fill(*this,6,5,19,1,Terrain::EMPTY);
 Painter::fill(*this,24,7,1,15,Terrain::EMPTY);
 Painter::fill(*this,7,24,18,1,Terrain::EMPTY);
 setEntrance(5+5*Width);
 setExit(25+24*Width);
 setRaw(entrance(),Terrain::ENTRANCE);
 setRaw(exit(),Terrain::EXIT);
 finishGeneration();
#if !defined(PD_PLATFORM_PS1)
  createMobs();
  createItems();
#else
  // First PS1 loot step: a simple heap validates the heap/map ownership path
  // independently from the broad random item catalogue.
  drop(std::unique_ptr<Item>(new Gold(25)), randomDropCell());
#endif
 return true;
}
bool RegularLevel::buildBossArena(){
 auto fill=[this](int left,int top,int width,int height,int terrain){for(int y=top;y<top+height;++y)for(int x=left;x<left+width;++x)setRaw(x,y,terrain);};
 const int cx=Width/2;
 if(depth_==5){
     fill(5,4,22,23,Terrain::EMPTY);
     fill(7,6,18,19,Terrain::WATER);
     fill(8,7,16,17,Terrain::EMPTY);
     // Rat King is hidden behind the same secret-door, chest and mimic setup
     // as the normal RatKingPainter path, which this hand-built boss floor skips.
     fill(5,19,6,6,Terrain::WALL);
     fill(6,20,4,4,Terrain::EMPTY_SP);
     setRaw(10+22*Width,Terrain::SECRET_DOOR);
     ++secretDoors_;
     const int ratChests[]={6+20*Width,9+20*Width,6+23*Width,9+23*Width};
     for(int cell:ratChests){
         Heap* chest=drop(Generator::random(Random::Int(2)?Generator::Category::Weapon:Generator::Category::Armor,depth_),cell);
         if(chest)chest->type=Heap::Type::Chest;
     }
     if(Heap* mimic=heapAt(ratChests[Random::Int(4)]))mimic->type=Heap::Type::Mimic;
     setEntrance(cx+25*Width);
     setExit(cx+4*Width);
     setRaw(entrance(),Terrain::ENTRANCE);
     setRaw(exit(),Terrain::EXIT);
     setRaw(cx+24*Width,Terrain::SIGN);
     return true;
 }
 if(depth_==10){fill(4,23,24,5,Terrain::EMPTY);fill(6,5,20,18,Terrain::EMPTY);fill(8,7,16,14,Terrain::INACTIVE_TRAP);for(int y=21;y<=23;++y)setRaw(cx,y,Terrain::EMPTY);bossArenaDoor_=cx+22*Width;setRaw(bossArenaDoor_,Terrain::LOCKED_DOOR);setEntrance(cx+5*Width);setExit(cx+27*Width);setRaw(entrance(),Terrain::ENTRANCE);setRaw(exit(),Terrain::EXIT);Heap*key=drop(std::unique_ptr<Item>(new IronKey(depth_)),cx+21*Width);if(key)key->type=Heap::Type::Chest;return true;}
 if(depth_==15){const int roomLeft=cx-2,roomRight=cx+2,roomTop=Height/2-2,roomBottom=Height/2+2;int topMost=Height,exitCell=0;for(int i=0;i<8;++i){int left=0,right=0,top=0,bottom=0;if(Random::Int(2)==0){left=Random::Int(1,roomLeft-3);right=roomRight+3;}else{left=roomLeft-3;right=Random::Int(roomRight+3,Width-1);}if(Random::Int(2)==0){top=Random::Int(2,roomTop-3);bottom=roomBottom+3;}else{top=roomLeft-3;bottom=Random::Int(roomTop+3,Height-1);}fill(left,top,right-left+1,bottom-top+1,Terrain::EMPTY);if(top<topMost){topMost=top;exitCell=Random::Int(left,right)+(top-1)*Width;}}setRaw(exitCell,Terrain::LOCKED_EXIT);for(int i=0;i<Length;++i)if(cell(i)==Terrain::EMPTY&&Random::Int(6)==0)setRaw(i,Terrain::INACTIVE_TRAP);fill(roomLeft-1,roomTop-1,7,7,Terrain::WALL);fill(roomLeft,roomTop+1,5,4,Terrain::EMPTY);for(int x=roomLeft;x<=roomRight;++x)setRaw(x,roomTop,Terrain::TOXIC_TRAP);bossArenaDoor_=Random::Int(roomLeft,roomRight)+(roomBottom+1)*Width;setRaw(bossArenaDoor_,Terrain::DOOR);setEntrance(Random::Int(roomLeft+1,roomRight-1)+Random::Int(roomTop+1,roomBottom-1)*Width);setExit(exitCell);setRaw(entrance(),Terrain::ENTRANCE);setRaw(exit(),Terrain::LOCKED_EXIT);const auto&waterPatch=Patch::generate(0.45f,6);for(int i=0;i<Length;++i)if(cell(i)==Terrain::EMPTY&&waterPatch[static_cast<std::size_t>(i)])setRaw(i,Terrain::WATER);for(int i=Width+1;i<Length-Width;++i)if(cell(i)==Terrain::EMPTY){int walls=(cell(i+1)==Terrain::WALL?1:0)+(cell(i-1)==Terrain::WALL?1:0)+(cell(i+Width)==Terrain::WALL?1:0)+(cell(i-Width)==Terrain::WALL?1:0);if(Random::Int(8)<=walls)setRaw(i,Terrain::EMPTY_DECO);}for(int i=0;i<Length;++i)if(cell(i)==Terrain::WALL&&Random::Int(8)==0)setRaw(i,Terrain::WALL_DECO);int sign=0;do{sign=Random::Int(roomLeft,roomRight)+Random::Int(roomTop,roomBottom)*Width;}while(sign==entrance());setRaw(sign,Terrain::SIGN);return true;}
 if(depth_==20){const int left=cx-3,top=2;fill(left,top,7,15,Terrain::EMPTY);fill(cx,top,1,15,Terrain::EMPTY_SP);for(int y=3;y<17;y+=2){setRaw(cx-2,y,Terrain::STATUE_SP);setRaw(cx+2,y,Terrain::STATUE_SP);}for(int x=cx-2;x<=cx+2;++x)setRaw(x,top+7,Terrain::EMPTY_SP);setRaw(cx-2,top+7,Terrain::PEDESTAL);setRaw(cx+2,top+7,Terrain::PEDESTAL);fill(left,18,7,3,Terrain::EMPTY);for(int y=18;y<21;++y){setRaw(left,y,Terrain::BOOKSHELF);setRaw(left+6,y,Terrain::BOOKSHELF);}setEntrance(left+Random::Int(5)+(19+Random::Int(2))*Width);setExit(cx+(top-1)*Width);setRaw(entrance(),Terrain::ENTRANCE);setRaw(exit(),Terrain::EXIT);bossArenaDoor_=cx+17*Width;setRaw(bossArenaDoor_,Terrain::DOOR);for(int i=0;i<Length;++i){if(cell(i)==Terrain::EMPTY&&Random::Int(10)==0)setRaw(i,Terrain::EMPTY_DECO);else if(cell(i)==Terrain::WALL&&Random::Int(8)==0)setRaw(i,Terrain::WALL_DECO);}setRaw(bossArenaDoor_+Width+1,Terrain::SIGN);return true;}
 if(depth_==25){
     // Yog's former random columns could leave the 3x3 entrance chamber
     // sealed off. Keep the five-column layout, but join every chamber with
     // an explicit central corridor before applying decoration.
     const int roomLeft=cx-1, roomTop=Height/2-1, corridorY=Height/2;
     for(int i=0;i<5;++i){
         const int left=2+i*6;
         fill(left,5,4,22,Terrain::EMPTY);
         for(int j=0;j<4;++j)if(Random::Int(2)==0)setRaw(left+j,Random::IntRange(6,25),Terrain::EMPTY_DECO);
     }
     // The full-width corridor deliberately overlaps the central 3x3 arena.
     fill(2,corridorY,28,1,Terrain::EMPTY);
     fill(roomLeft,roomTop,3,3,Terrain::EMPTY);
     const int entrance=cx+corridorY*Width;
     const int exitCell=27+5*Width;
     setEntrance(entrance);setExit(exitCell);
     setRaw(entrance,Terrain::ENTRANCE);setRaw(exitCell,Terrain::LOCKED_EXIT);
     const auto&waterPatch=Patch::generate(0.45f,6);
     for(int i=0;i<Length;++i)if(cell(i)==Terrain::EMPTY&&i!=entrance&&waterPatch[static_cast<std::size_t>(i)])setRaw(i,Terrain::WATER);
     // Water must never cut the only route out of the initial Yog chamber.
     fill(2,corridorY,28,1,Terrain::EMPTY);
     fill(roomLeft,roomTop,3,3,Terrain::EMPTY);
     setRaw(entrance,Terrain::ENTRANCE);setRaw(exitCell,Terrain::LOCKED_EXIT);
     for(int i=0;i<Length;++i)if(cell(i)==Terrain::EMPTY&&Random::Int(10)==0)setRaw(i,Terrain::EMPTY_DECO);
     return true;
 }
 return false;
}
void RegularLevel::sealBossEntrance(){
 if(bossArenaDoor_!=0&&cell(bossArenaDoor_)==Terrain::DOOR){setRaw(bossArenaDoor_,Terrain::LOCKED_DOOR);buildFlagMaps();return;}
 if(!bossLevel(depth_)||sealedEntrance_!=0||entrance()==0)return;
 sealedEntrance_=bossEntranceCell_?bossEntranceCell_:entrance();setRaw(sealedEntrance_,Terrain::WATER);setEntrance(0);buildFlagMaps();
}
void RegularLevel::unsealBossEntrance(){
 if(bossArenaDoor_!=0&&cell(bossArenaDoor_)==Terrain::LOCKED_DOOR){setRaw(bossArenaDoor_,Terrain::DOOR);buildFlagMaps();return;}
 if(sealedEntrance_==0&&bossEntranceCell_!=0&&entrance()==0&&cell(bossEntranceCell_)==Terrain::WATER)sealedEntrance_=bossEntranceCell_;
 if(sealedEntrance_==0)return;
 setEntrance(sealedEntrance_);setRaw(sealedEntrance_,Terrain::ENTRANCE);sealedEntrance_=0;buildFlagMaps();
}

void RegularLevel::press(int cell,Char* ch){
 Level::press(cell,ch);
 if(!dynamic_cast<Hero*>(ch)||bossArenaActivated_)return;
 if(depth_==10&&cell/Width>=23)activateTenguArena();
 else if(depth_==15&&((cell%Width)<Width/2-3||(cell%Width)>Width/2+3||cell/Width<Height/2-3||cell/Width>Height/2+3))activateDM300Arena();
 else if(depth_==25&&cell!=entrance())activateYogArena();
 else if(depth_==20&&cell/Width<17)activateKingArena();
}

bool RegularLevel::activateTenguArena(){
 Dungeon*game=dungeonContext();
 if(!game)return false;
 for(const auto&mob:mobs())if(ZoneBoss*boss=dynamic_cast<ZoneBoss*>(mob.get()))if(boss->kind()==ZoneBossKind::Tengu){boss->setState(Mob::State::Hunting);boss->sealArena();bossArenaActivated_=true;sealBossEntrance();return true;}
 std::vector<int>cells;
 for(int cell=0;cell<Length;++cell)if(cell/Width>=23&&(passable(cell)||avoid(cell))&&!game->scheduler().findChar(cell))cells.push_back(cell);
 if(cells.empty())return false;
 std::unique_ptr<ZoneBoss>tengu(new ZoneBoss(ZoneBossKind::Tengu));ZoneBoss*rawTengu=tengu.get();tengu->pos=cells[static_cast<std::size_t>(Random::Int(static_cast<int>(cells.size())))];tengu->setState(Mob::State::Hunting);tengu->sealArena();tengu->bind(game);addMob(std::move(tengu));game->scheduler().add(rawTengu);
 // The Tengu always spawns on an ordinary arena tile.  Calling mobPress here
 // re-entered trap processing while the arena and scheduler were being changed,
 // which could stall the first turn on floor 10.
 bossArenaActivated_=true;
 sealBossEntrance();
 return true;
}

bool RegularLevel::activateDM300Arena(){
 Dungeon*game=dungeonContext();
 if(!game)return false;
 for(const auto&mob:mobs())if(const ZoneBoss*boss=dynamic_cast<const ZoneBoss*>(mob.get()))if(boss->kind()==ZoneBossKind::DM300){bossArenaActivated_=true;return true;}
 std::vector<int>cells;
 for(int cell=0;cell<Length;++cell)if(((cell%Width)<Width/2-3||(cell%Width)>Width/2+3||cell/Width<Height/2-3||cell/Width>Height/2+3)&&(passable(cell)||avoid(cell))&&!game->visible()[static_cast<std::size_t>(cell)]&&!game->scheduler().findChar(cell))cells.push_back(cell);
 if(cells.empty())return false;
 std::unique_ptr<ZoneBoss>dm300(new ZoneBoss(ZoneBossKind::DM300));ZoneBoss*rawDM300=dm300.get();dm300->pos=cells[static_cast<std::size_t>(Random::Int(static_cast<int>(cells.size())))];dm300->setState(Mob::State::Hunting);dm300->sealArena();dm300->bind(game);addMob(std::move(dm300));game->scheduler().add(rawDM300);
 bossArenaActivated_=true;
 sealBossEntrance();
 return true;
}

bool RegularLevel::activateYogArena(){
 Dungeon*game=dungeonContext();
 if(!game)return false;
 for(const auto&mob:mobs())if(const ZoneBoss*boss=dynamic_cast<const ZoneBoss*>(mob.get()))if(boss->kind()==ZoneBossKind::Yog){bossArenaActivated_=true;return true;}
 std::vector<int>cells;
 for(int cell=0;cell<Length;++cell){
  if((!passable(cell)&&!avoid(cell))||game->visible()[static_cast<std::size_t>(cell)]||game->scheduler().findChar(cell))continue;
  int open=0;for(int offset:Neighbours8){const int next=cell+offset;if(inBoundsCell(next)&&(passable(next)||avoid(next))&&!game->scheduler().findChar(next))++open;}
  if(open>=2)cells.push_back(cell);
 }
 if(cells.empty())return false;
 const int yogCell=cells[static_cast<std::size_t>(Random::Int(static_cast<int>(cells.size())))];
 std::unique_ptr<ZoneBoss>yog(new ZoneBoss(ZoneBossKind::Yog));
 ZoneBoss*rawYog=yog.get();yog->pos=yogCell;yog->bind(game);yog->activateYogArena();addMob(std::move(yog));game->scheduler().add(rawYog);
 std::vector<int>adjacent;
 for(int offset:Neighbours8){const int cell=yogCell+offset;if(inBoundsCell(cell)&&(passable(cell)||avoid(cell))&&!game->scheduler().findChar(cell))adjacent.push_back(cell);}
 for(ZoneMobKind kind:{ZoneMobKind::RottingFist,ZoneMobKind::BurningFist}){
  const int index=Random::Int(static_cast<int>(adjacent.size()));const int cell=adjacent[static_cast<std::size_t>(index)];adjacent.erase(adjacent.begin()+index);
  std::unique_ptr<ZoneMob>fist(new ZoneMob(kind));ZoneMob*rawFist=fist.get();fist->pos=cell;fist->bind(game);addMob(std::move(fist));game->scheduler().add(rawFist);
 }
 bossArenaActivated_=true;
 sealBossEntrance();
 return true;
}

bool RegularLevel::activateKingArena(){
 Dungeon*game=dungeonContext();
 if(!game)return false;
 for(const auto&mob:mobs())if(const ZoneBoss*boss=dynamic_cast<const ZoneBoss*>(mob.get()))if(boss->kind()==ZoneBossKind::King){bossArenaActivated_=true;return true;}
 std::vector<int>cells;
 for(int cell=0;cell<Length;++cell)if(cell/Width<17&&(passable(cell)||avoid(cell))&&!game->scheduler().findChar(cell))cells.push_back(cell);
 if(cells.empty())return false;
 int kingCell=cells[static_cast<std::size_t>(Random::Int(static_cast<int>(cells.size())))];for(int attempt=0;attempt<20&&game->visible()[static_cast<std::size_t>(kingCell)];++attempt)kingCell=cells[static_cast<std::size_t>(Random::Int(static_cast<int>(cells.size())))];
 std::unique_ptr<ZoneBoss>king(new ZoneBoss(ZoneBossKind::King));ZoneBoss*rawKing=king.get();king->pos=kingCell;king->setState(Mob::State::Hunting);king->sealArena();king->bind(game);addMob(std::move(king));game->scheduler().add(rawKing);
 bossArenaActivated_=true;
 sealBossEntrance();
 return true;
}
void RegularLevel::initRooms(){
 std::vector<Rect> cells; split(Rect(0,0,Width-1,Height-1),cells);
 for(const Rect&r:cells) if(r.width()>=3&&r.height()>=3) rooms_.push_back(std::make_unique<Room>(r.left,r.top,r.right,r.bottom));
 for(std::size_t i=0;i<rooms_.size();++i)for(std::size_t j=i+1;j<rooms_.size();++j)rooms_[i]->addNeighbour(rooms_[j].get());
}
void RegularLevel::split(const Rect&r,std::vector<Rect>&out){
 const int minRoomSize=7,maxRoomSize=9;
 int w=r.width(),h=r.height();
 if(w>maxRoomSize && h<minRoomSize){
  int vw=Random::Int(r.left+3,r.right-3);
  split(Rect(r.left,r.top,vw,r.bottom),out);split(Rect(vw,r.top,r.right,r.bottom),out);
 } else if(h>maxRoomSize && w<minRoomSize){
  int vh=Random::Int(r.top+3,r.bottom-3);
  split(Rect(r.left,r.top,r.right,vh),out);split(Rect(r.left,vh,r.right,r.bottom),out);
 } else if((Random::Float() <= (float)(minRoomSize*minRoomSize)/(float)r.square() && w<=maxRoomSize && h<=maxRoomSize) || w<minRoomSize || h<minRoomSize){
  out.push_back(r);
 } else if(Random::Float() < (float)(w-2)/(float)(w+h-4)){
  int vw=Random::Int(r.left+3,r.right-3);
  split(Rect(r.left,r.top,vw,r.bottom),out);split(Rect(vw,r.top,r.right,r.bottom),out);
 } else {
  int vh=Random::Int(r.top+3,r.bottom-3);
  split(Rect(r.left,r.top,r.right,vh),out);split(Rect(r.left,vh,r.right,r.bottom),out);
 }
}
bool RegularLevel::build(){
 if(depth_==21)return buildLastShop();
 initRooms(); if(rooms_.size()<8)return false;
 std::vector<Room*> nodes;for(auto&u:rooms_)nodes.push_back(u.get());
 const int minDistance=static_cast<int>(std::floor(std::sqrt(static_cast<float>(rooms_.size()))));
 std::vector<Room*> candidates;for(Room*r:nodes)if(r->width()>=4&&r->height()>=4)candidates.push_back(r);
 if(candidates.size()<2)return false;
 bool ok=false;
 for(int tries=0;tries<10 && !ok;++tries){
  roomEntrance_=Random::element(candidates);
  do{roomExit_=Random::element(candidates);}while(roomExit_==roomEntrance_);
  Graph::setPrice(nodes,1.0f);Graph::buildDistanceMap(nodes,roomExit_);
  ok=std::isfinite(roomEntrance_->distance)&&roomEntrance_->distance>=minDistance;
 }
 if(!ok)return false;
 roomEntrance_->type=Room::Type::Entrance;
 roomExit_->type=bossLevel(depth_)?Room::Type::BossExit:Room::Type::Exit;

 std::vector<Room*> connected;
 connected.push_back(roomEntrance_);
 auto addConnected=[&](Room* r){if(std::find(connected.begin(),connected.end(),r)==connected.end())connected.push_back(r);};

 Graph::buildDistanceMap(nodes,roomExit_);
 auto path1=Graph::buildPath(nodes,roomEntrance_,roomExit_); if(path1.empty())return false;
 Room* prev=roomEntrance_;for(Room*r:path1){prev->connect(r);prev=r;addConnected(r);}
 Graph::setPrice(path1,roomEntrance_->distance);
 Graph::buildDistanceMap(nodes,roomExit_);
 auto path2=Graph::buildPath(nodes,roomEntrance_,roomExit_);
 prev=roomEntrance_;for(Room*r:path2){prev->connect(r);prev=r;addConnected(r);}

 int target=static_cast<int>(rooms_.size()*Random::Float(0.50f,0.70f));
 int guard=0;
 while(static_cast<int>(connected.size())<target && guard++<1000){
  Room* cr=Random::element(connected);if(!cr||cr->neighbours.empty())continue;
  Room* other=Random::element(cr->neighbours);if(!other)continue;
  if(std::find(connected.begin(),connected.end(),other)==connected.end()){
   cr->connect(other);addConnected(other);
  }
 }
 if(depth_==6||depth_==11||depth_==16){
  Room* shop=nullptr;
  for(const auto& entry:roomEntrance_->connected){
   Room* candidate=entry.first;
   if(candidate&&candidate->connected.size()==1&&candidate->width()>=5&&candidate->height()>=5){shop=candidate;break;}
  }
  if(!shop)return false;
  shop->type=Room::Type::Shop;
 }
 assignRoomType();paint();paintWater();paintGrass();placeTraps();return entrance()!=0&&exit()!=0;
}

bool RegularLevel::buildLastShop(){
 initRooms();
 if(rooms_.size()<2)return false;
 std::vector<Room*> all;
 for(auto& room:rooms_)all.push_back(room.get());
 const int minDistance=static_cast<int>(std::floor(std::sqrt(static_cast<float>(rooms_.size()))));
 bool found=false;
 for(int retry=0;retry<=10&&!found;++retry){
  int innerRetry=0;
  do { if(innerRetry++>10)return false; roomEntrance_=Random::element(all); }
  while(!roomEntrance_||roomEntrance_->width()<4||roomEntrance_->height()<4);
  innerRetry=0;
  do { if(innerRetry++>10)return false; roomExit_=Random::element(all); }
  while(!roomExit_||roomExit_==roomEntrance_||roomExit_->width()<6||roomExit_->height()<6||roomExit_->top==0);
  Graph::buildDistanceMap(all,roomExit_);
  const auto path=Graph::buildPath(all,roomEntrance_,roomExit_);
  found=static_cast<int>(path.size())>=minDistance;
 }
 if(!found)return false;
 roomEntrance_->type=Room::Type::Entrance;
 roomExit_->type=Room::Type::Exit;
 Graph::buildDistanceMap(all,roomExit_);
 const auto firstPath=Graph::buildPath(all,roomEntrance_,roomExit_);
 Graph::setPrice(firstPath,roomEntrance_->distance);
 Graph::buildDistanceMap(all,roomExit_);
 const auto path=Graph::buildPath(all,roomEntrance_,roomExit_);
 Room* previous=roomEntrance_;
 for(Room* room:path){previous->connect(room);previous=room;}
 Room* shop=nullptr;
 int shopSquare=0;
 for(Room* room:all){
  if(room->type!=Room::Type::Null||room->connected.empty())continue;
  room->type=Room::Type::Passage;
  if(room->square()>shopSquare){shop=room;shopSquare=room->square();}
 }
 if(!shop||shopSquare<30)return false;
 const bool impComplete=dungeonContext()&&dungeonContext()->questState(3)==2;
 shop->type=impComplete?Room::Type::Shop:Room::Type::Standard;
 paint();paintWater();paintGrass();
 return entrance()!=0&&exit()!=0;
}
void RegularLevel::assignRoomType(){
 int specialRooms=0;
 std::vector<Room::Type> specials={Room::Type::Armory,Room::Type::WeakFloor,Room::Type::MagicWell,Room::Type::Crypt,Room::Type::Pool,Room::Type::Garden,Room::Type::Library,Room::Type::Treasury,Room::Type::Traps,Room::Type::Storage,Room::Type::Statue,Room::Type::Laboratory,Room::Type::Vault,Room::Type::Altar};
 for(auto&u:rooms_){
  Room&r=*u;
  if(r.type==Room::Type::Null && r.connected.size()==1 && !bossLevel(depth_) && !specials.empty() &&
     r.width()>3 && r.height()>3 && Random::Int(specialRooms*specialRooms+2)==0){
   std::size_t index=0;
   if(pitRoomNeeded_){r.type=Room::Type::Pit;pitRoomNeeded_=false;specials.erase(std::remove(specials.begin(),specials.end(),Room::Type::WeakFloor),specials.end());++specialRooms;continue;}
   if(depth_%5==2){auto it=std::find(specials.begin(),specials.end(),Room::Type::Laboratory);if(it!=specials.end())index=static_cast<std::size_t>(it-specials.begin());else index=static_cast<std::size_t>(Random::Int(static_cast<int>(specials.size())));}
   else {const int n=static_cast<int>(specials.size());index=static_cast<std::size_t>(std::min(Random::Int(n),Random::Int(n)));}
   r.type=specials[index];if(r.type==Room::Type::WeakFloor)weakFloorCreated_=true;specials.erase(specials.begin()+static_cast<std::ptrdiff_t>(index));++specialRooms;
  } else if(r.type==Room::Type::Null && r.connected.size()==1 && Random::Int(2)==0){
   std::vector<Room*> candidates;
   for(Room*n:r.neighbours){
    if(!n)continue;
    if(r.connected.find(n)==r.connected.end() && n->type!=Room::Type::Pit)candidates.push_back(n);
   }
   if(candidates.size()>1)r.connect(Random::element(candidates));
  }
 }
 int count=0;
 for(auto&u:rooms_){
  Room&r=*u;if(r.type!=Room::Type::Null)continue;
  int connections=static_cast<int>(r.connected.size());
  if(connections==0)continue;
  if(Random::Int(connections*connections)==0){r.type=Room::Type::Standard;++count;}
  else r.type=Room::Type::Tunnel;
 }
 std::vector<Room*> all;for(auto&u:rooms_)all.push_back(u.get());
 int guard=0;
 while(count<4 && guard++<1000){
  Room*r=Random::element(all);
  if(r&&r->type==Room::Type::Tunnel){r->type=Room::Type::Standard;++count;}
 }
 if(depth_>11&&depth_<15){
  if(Dungeon*game=dungeonContext())if(!game->blacksmithSpawned()&&Random::Int(15-depth_)==0)for(Room*room:all){
   if(room&&room->type==Room::Type::Standard&&room->width()>4&&room->height()>4){room->type=Room::Type::Blacksmith;game->restoreBlacksmithSpawned(true);break;}
  }
 }
}
void RegularLevel::paint(){
 for(auto&u:rooms_){
  Room&r=*u;
  if(r.type!=Room::Type::Null){placeDoors(r);paintRoom(r);}
  else if(feeling()==Feeling::Chasm && Random::Int(2)==0)Painter::fill(*this,r,Terrain::WALL);
 }
 for(auto&u:rooms_)paintDoors(*u);
}
void RegularLevel::placeDoors(Room&room){
 for(Room*n:room.neighbours){
  auto it=room.connected.find(n);
  if(it==room.connected.end() || !n)continue;
  if(it->second)continue;
  Rect inter=room.intersect(*n);
  std::shared_ptr<Room::Door> door;
  if(inter.width()==0){
   door=std::make_shared<Room::Door>(inter.left,Random::Int(inter.top+1,inter.bottom));
  } else {
   door=std::make_shared<Room::Door>(Random::Int(inter.left+1,inter.right),inter.top);
  }
  room.connected[n]=door;
  n->connected[&room]=door;
 }
}
void RegularLevel::paintRoom(Room&r){
 switch(r.type){
  case Room::Type::Entrance:EntrancePainter::paint(*this,r);break;
  case Room::Type::Exit:ExitPainter::paint(*this,r);break;
  case Room::Type::BossExit:BossExitPainter::paint(*this,r);break;
  case Room::Type::Armory:case Room::Type::WeakFloor:case Room::Type::MagicWell:
  case Room::Type::Crypt:case Room::Type::Pool:case Room::Type::Garden:
  case Room::Type::Library:case Room::Type::Treasury:case Room::Type::Traps:
  case Room::Type::Storage:case Room::Type::Statue:case Room::Type::Laboratory:
  case Room::Type::Vault:case Room::Type::Altar:case Room::Type::Pit:case Room::Type::RatKing:
  case Room::Type::Blacksmith:
   SpecialRoomPainter::paint(*this,r);break;
  case Room::Type::Shop:
   Painter::fill(*this,r,Terrain::WALL);
   Painter::fill(*this,r,1,Terrain::EMPTY_SP);
   for(const auto& door:r.doors())if(door)door->set(Room::Door::Type::Regular);
   break;
  case Room::Type::Standard:StandardPainter::paint(*this,r,depth_);break;
  case Room::Type::Tunnel:
   TunnelPainter::paint(*this,r);break;
  case Room::Type::Passage:PassagePainter::paint(*this,r);break;
  default: Painter::fill(*this,r,Terrain::WALL);break;
 }
}
bool RegularLevel::joinRooms(Room&r,Room&n){
 if(r.type!=Room::Type::Standard || n.type!=Room::Type::Standard)return false;
 Rect w=r.intersect(n);
 if(w.left==w.right){
  if(w.bottom-w.top<3)return false;
  if(w.height()==std::max(r.height(),n.height()))return false;
  if(r.width()+n.width()>maxRoomSize())return false;
  w.top+=1;w.right++;
  Painter::fill(*this,w.left,w.top,1,w.height(),Terrain::EMPTY);
 } else {
  if(w.right-w.left<3)return false;
  if(w.width()==std::max(r.width(),n.width()))return false;
  if(r.height()+n.height()>maxRoomSize())return false;
  w.left+=1;w.bottom++;
  Painter::fill(*this,w.left,w.top,w.width(),1,Terrain::EMPTY);
 }
 return true;
}
void RegularLevel::paintDoors(Room&room){
 for(auto&kv:room.connected){
  Room*other=kv.first;auto d=kv.second;if(!other||!d)continue;
  if(joinRooms(room,*other))continue;
  int t=Terrain::EMPTY;
  switch(d->type){
   case Room::Door::Type::Empty:t=Terrain::EMPTY;break;
   case Room::Door::Type::Tunnel:t=tunnelTile();break;
   case Room::Door::Type::Regular:
    if(depth_<=1)t=Terrain::DOOR;
    else {
     bool secret=(depth_<6?Random::Int(12-depth_):Random::Int(6))==0;
     t=secret?Terrain::SECRET_DOOR:Terrain::DOOR;if(secret)++secretDoors_;
    }
    break;
   case Room::Door::Type::Unlocked:t=Terrain::DOOR;break;
   case Room::Door::Type::Hidden:t=Terrain::SECRET_DOOR;++secretDoors_;break;
   case Room::Door::Type::Barricade:t=Random::Int(3)==0?Terrain::BOOKSHELF:Terrain::BARRICADE;break;
   case Room::Door::Type::Locked:t=Terrain::LOCKED_DOOR;break;
  }
  setRaw(d->x,d->y,t);
 }
}
void RegularLevel::paintWater(){
 const auto&patch=Patch::generate(water()/100.0f,5);
 for(int i=0;i<Length;++i)if(patch[i]&&cell(i)==Terrain::EMPTY)setRaw(i,Terrain::WATER);
}
void RegularLevel::paintGrass(){
 auto grassPatch=Patch::generate(grass()/100.0f,4);
 if(feeling()==Feeling::Grass){
  for(auto&u:rooms_){
   Room&r=*u;
   if(r.type!=Room::Type::Null&&r.type!=Room::Type::Passage&&r.type!=Room::Type::Tunnel){
    grassPatch[(r.left+1)+(r.top+1)*Width]=true;
    grassPatch[(r.right-1)+(r.top+1)*Width]=true;
    grassPatch[(r.left+1)+(r.bottom-1)*Width]=true;
    grassPatch[(r.right-1)+(r.bottom-1)*Width]=true;
   }
  }
 }
 for(int i=Width+1;i<Length-Width-1;++i){
  if(cell(i)==Terrain::EMPTY&&grassPatch[i]){
   int count=1;for(int n:Neighbours8)if(grassPatch[i+n])++count;
   setRaw(i,Random::Float()<count/12.0f?Terrain::HIGH_GRASS:Terrain::GRASS);
  }
 }
}
int RegularLevel::nTraps() const{
 if(depth_<=1)return 0;
 return Random::Int(1,static_cast<int>(rooms_.size())+depth_);
}
void RegularLevel::placeTraps(){
 const int count=nTraps();
 for(int i=0;i<count;++i){
  const int pos=Random::Int(Length);
  if(cell(pos)!=Terrain::EMPTY)continue;
  switch(Random::Int(8)){
   case 0:setRaw(pos,Terrain::SECRET_TOXIC_TRAP);break;
   case 1:setRaw(pos,Terrain::SECRET_FIRE_TRAP);break;
   case 2:setRaw(pos,Terrain::SECRET_PARALYTIC_TRAP);break;
   case 3:setRaw(pos,Terrain::SECRET_POISON_TRAP);break;
   case 4:setRaw(pos,Terrain::SECRET_ALARM_TRAP);break;
   case 5:setRaw(pos,Terrain::SECRET_LIGHTNING_TRAP);break;
   case 6:setRaw(pos,Terrain::SECRET_GRIPPING_TRAP);break;
   case 7:setRaw(pos,Terrain::SECRET_SUMMONING_TRAP);break;
  }
 }
}

int RegularLevel::nMobs() const {
    // Original RegularLevel.nMobs():
    // 2 + Dungeon.depth % 5 + Random.Int(3)
    if (bossLevel(depth_)) return 1;
    return 2 + depth_ % 5 + Random::Int(3);
}

Room* RegularLevel::randomRoom(
    Room::Type type,
    int tries
) {
    std::vector<Room*> values;

    for (auto& room : rooms_) {
        values.push_back(room.get());
    }

    for (int i = 0; i < tries; ++i) {
        Room* room = Random::element(values);

        if (room && room->type == type) {
            return room;
        }
    }

    return nullptr;
}

int RegularLevel::randomRespawnCell() {
    if(rooms_.empty()){
        for(int attempt=0;attempt<40;++attempt){
            const int cell=Random::Int(Length);
            if(passable(cell)&&mobAt(cell)==nullptr)return cell;
        }
        return -1;
    }
    int count = 0;

    while (true) {
        if (++count > 10) {
            return -1;
        }

        Room* room =
            randomRoom(
                Room::Type::Standard,
                10
            );

        if (!room) {
            continue;
        }

        const Point point = room->random();
        const int cell =
            point.x +
            point.y * Width;

        if (passable(cell) &&
            mobAt(cell) == nullptr) {
            return cell;
        }
    }
}

int RegularLevel::pitCell() const {
    for (const auto& room : rooms_) {
        if (!room || room->type != Room::Type::Pit) continue;
        for (int attempt = 0; attempt < 20; ++attempt) {
            const Point point = room->random();
            const int cell = point.x + point.y * Width;
            if (inBoundsCell(cell) && passable(cell)) return cell;
        }
    }
    return -1;
}

bool RegularLevel::isWeakFloorCell(int cell) const {
    if (!inBoundsCell(cell)) return false;
    const Point point(cell % Width, cell / Width);
    for (const auto& room : rooms_) {
        if (room && room->type == Room::Type::WeakFloor && room->insideCell(point)) return true;
    }
    return false;
}

int RegularLevel::randomDestination() {
    if (rooms_.empty()) return Level::randomDestination();

    while (true) {
        std::vector<Room*> values;

        for (auto& room : rooms_) {
            values.push_back(room.get());
        }

        Room* room =
            Random::element(values);

        if (!room) {
            continue;
        }

        const Point point = room->random();
        const int cell =
            point.x +
            point.y * Width;

        if (passable(cell)) {
            return cell;
        }
    }
}


int RegularLevel::randomDropCell() {
    for (int attempt = 0; attempt < 100; ++attempt) {
        Room* room = randomRoom(Room::Type::Standard, 1);
        if (!room) break;
        const Point p = room->random();
        const int cell = p.x + p.y * Width;
        if (passable(cell) && !heapAt(cell) && !mobAt(cell)) return cell;
    }
    for (int attempt = 0; attempt < 100; ++attempt) {
        const int cell = Random::Int(Length);
        if (passable(cell) && !heapAt(cell) && !mobAt(cell)) return cell;
    }
    return randomDestination();
}

void RegularLevel::createItems() {
    // LastShopLevel never inherits RegularLevel's food/random-item rolls.
    // Before the Imp quest is complete it contains only restored Bones; after
    // completion the code below adds the dedicated shop range instead.
    if (depth_ == 21 && (!dungeonContext() || dungeonContext()->questState(3) != 2)) {
        itemsToSpawn_.clear();
        if (Dungeon* game = dungeonContext()) {
            std::unique_ptr<Item> bones = game->takeBones(depth_);
            if (bones) {
                Heap* heap = drop(std::move(bones), randomDropCell());
                if (heap) heap->type = Heap::Type::Skeleton;
            }
        }
        return;
    }

    const bool lastShop = depth_ == 21;
    int nItems = lastShop ? 0 : 3;
    while (!lastShop && Random::Float() < 0.4f) ++nItems;

    for (int i=0;i<nItems;++i) {
        Heap::Type type = Heap::Type::Heap;
        switch (Random::Int(20)) {
            case 0: type = Heap::Type::Skeleton; break;
            case 1: case 2: case 3: case 4: type = Heap::Type::Chest; break;
            case 5: type = depth_ > 1 ? Heap::Type::Mimic : Heap::Type::Chest; break;
            default: type = Heap::Type::Heap; break;
        }

        const bool closedChest=type==Heap::Type::Chest||type==Heap::Type::LockedChest||type==Heap::Type::CrystalChest||type==Heap::Type::Mimic;
        std::unique_ptr<Item> item = closedChest
            ? std::unique_ptr<Item>(new SpecialItem(SpecialKind::ChestPlaceholder))
            : Generator::random(depth_);
        const int cell = randomDropCell();
        if (!item) continue; // category exists upstream but its concrete family is not ported yet
        Heap* heap = drop(std::move(item), cell);
        if (heap) heap->type = type;
    }

    if (!lastShop) {
        for (auto& item : itemsToSpawn_) {
            if (!item) continue;
            int cell = randomDropCell();
            if (dynamic_cast<ScrollOfUpgrade*>(item.get())) {
                while (this->cell(cell) == Terrain::FIRE_TRAP ||
                       this->cell(cell) == Terrain::SECRET_FIRE_TRAP) {
                    cell = randomDropCell();
                }
            }
            Heap* heap = drop(std::move(item), cell);
            if (heap) heap->type = Heap::Type::Heap;
        }
    }
    itemsToSpawn_.clear();
    const bool impShop=depth_==21&&dungeonContext()&&dungeonContext()->questState(3)==2;
    const bool regularShop=depth_==6||depth_==11||depth_==16;
    const bool hasShop=regularShop||impShop;
    if(hasShop){
        std::vector<int> shopCells;
        for(const auto& room:rooms_){
            if(!room || room->type!=Room::Type::Shop) continue;
            for(int x=room->left+1;x<room->right;++x)shopCells.push_back((room->top+1)*Width+x);
            for(int y=room->top+2;y<room->bottom;++y)shopCells.push_back(y*Width+room->right-1);
            for(int x=room->right-2;x>=room->left+1;--x)shopCells.push_back((room->bottom-1)*Width+x);
            for(int y=room->bottom-2;y>room->top+1;--y)shopCells.push_back(y*Width+room->left+1);
            break;
        }
        std::size_t nextShopCell=0;
        auto shopCell=[this,&shopCells,&nextShopCell](){
            while(nextShopCell<shopCells.size()){
                const int cell=shopCells[nextShopCell++];
                if(passable(cell) && !heapAt(cell) && !mobAt(cell)) return cell;
            }
            for(const auto& room:rooms_) {
                if(!room || room->type!=Room::Type::Shop) continue;
                for(int attempt=0;attempt<30;++attempt) {
                    const Point point=room->random(1);
                    const int cell=point.y*Width+point.x;
                    if(passable(cell) && !heapAt(cell) && !mobAt(cell)) return cell;
                }
            }
            return randomDropCell();
        };
        std::vector<std::unique_ptr<Item>> shop;
        auto identified=[](std::unique_ptr<Item> item){if(item)item->identify();return item;};
        if(depth_==6){
            shop.push_back(identified(std::unique_ptr<Item>(new CatalogMelee(Random::Int(2)==0?MeleeKind::Quarterstaff:MeleeKind::Spear))));
            shop.push_back(identified(std::unique_ptr<Item>(new CatalogArmor(ArmorKind::Leather))));
            shop.push_back(std::unique_ptr<Item>(new SeedPouch()));
            shop.push_back(std::unique_ptr<Item>(new SpecialItem(SpecialKind::Weightstone)));
        }else if(depth_==11){
            shop.push_back(identified(std::unique_ptr<Item>(new CatalogMelee(Random::Int(2)==0?MeleeKind::Sword:MeleeKind::Mace))));
            shop.push_back(identified(std::unique_ptr<Item>(new CatalogArmor(ArmorKind::Mail))));
            shop.push_back(std::unique_ptr<Item>(new ScrollHolder()));
            shop.push_back(std::unique_ptr<Item>(new SpecialItem(SpecialKind::Weightstone)));
        }else if(depth_==16){
            shop.push_back(identified(std::unique_ptr<Item>(new CatalogMelee(Random::Int(2)==0?MeleeKind::Longsword:MeleeKind::BattleAxe))));
            shop.push_back(identified(std::unique_ptr<Item>(new CatalogArmor(ArmorKind::Scale))));
            shop.push_back(std::unique_ptr<Item>(new WandHolster()));
            shop.push_back(std::unique_ptr<Item>(new SpecialItem(SpecialKind::Weightstone)));
        }else{
            const int choice=Random::Int(3);
            if(choice==0)shop.push_back(identified(std::unique_ptr<Item>(new CatalogMelee(MeleeKind::Glaive))));
            else if(choice==1)shop.push_back(identified(std::unique_ptr<Item>(new CatalogMelee(MeleeKind::WarHammer))));
            else shop.push_back(identified(std::unique_ptr<Item>(new CatalogArmor(ArmorKind::Plate))));
            shop.push_back(std::unique_ptr<Item>(new SpecialItem(SpecialKind::Torch)));
            shop.push_back(std::unique_ptr<Item>(new SpecialItem(SpecialKind::Torch)));
        }
        shop.push_back(std::unique_ptr<Item>(new PotionOfHealing()));
        for(int i=0;i<3;++i)shop.push_back(Generator::random(Generator::Category::Potion,depth_));
        shop.push_back(std::unique_ptr<Item>(new Scroll(ScrollKind::Identify)));
        shop.push_back(std::unique_ptr<Item>(new Scroll(ScrollKind::RemoveCurse)));
        shop.push_back(std::unique_ptr<Item>(new Scroll(ScrollKind::MagicMapping)));
        shop.push_back(Generator::random(Generator::Category::Scroll,depth_));
        shop.push_back(std::unique_ptr<Item>(new PreparedFood(PreparedFoodKind::OverpricedRation)));
        shop.push_back(std::unique_ptr<Item>(new PreparedFood(PreparedFoodKind::OverpricedRation)));
        shop.push_back(std::unique_ptr<Item>(new SpecialItem(SpecialKind::Ankh)));
        for(std::size_t i=shop.size();i>1;--i)std::swap(shop[i-1],shop[static_cast<std::size_t>(Random::Int(static_cast<int>(i)))]);
        if(!shopCells.empty()){
            Room* shopRoom=nullptr;
            for(const auto& room:rooms_)if(room&&room->type==Room::Type::Shop){shopRoom=room.get();break;}
            std::size_t entranceCell=0;
            if(shopRoom){
                const auto doors=shopRoom->doors();
                if(!doors.empty()&&doors.front())entranceCell=static_cast<std::size_t>(doors.front()->y*Width+doors.front()->x);
            }
            std::size_t closest=0;
            int distance=std::numeric_limits<int>::max();
            for(std::size_t i=0;i<shopCells.size();++i){
                const int dx=std::abs(shopCells[i]%Width-static_cast<int>(entranceCell)%Width);
                const int dy=std::abs(shopCells[i]/Width-static_cast<int>(entranceCell)/Width);
                if(dx+dy<distance){distance=dx+dy;closest=i;}
            }
            const std::size_t start=(closest+(shopCells.size()-std::min(shopCells.size(),shop.size()))/2)%shopCells.size();
            std::rotate(shopCells.begin(),shopCells.begin()+static_cast<std::ptrdiff_t>(start),shopCells.end());
        }
        for(auto& item:shop){if(!item)continue;Heap*heap=drop(std::move(item),shopCell());if(heap)heap->type=Heap::Type::ForSale;}
    }
    if(Dungeon*game=dungeonContext()){std::unique_ptr<Item>bones=game->takeBones(depth_);if(bones){Heap*heap=drop(std::move(bones),randomDropCell());if(heap)heap->type=Heap::Type::Skeleton;}}
}

void RegularLevel::createMobs() {
    if(depth_==10){
        // Register Tengu during level creation, while the scheduler is still
        // assembled by Dungeon.  Spawning an actor from Hero::press caused a
        // scheduler mutation exactly when entering the lower arena.
        std::vector<int> cells;
        for(int cell=0;cell<Length;++cell)if(cell/Width>=23&&(passable(cell)||avoid(cell))&&!mobAt(cell))cells.push_back(cell);
        if(!cells.empty()){
            std::unique_ptr<ZoneBoss> tengu(new ZoneBoss(ZoneBossKind::Tengu));
            tengu->pos=cells[static_cast<std::size_t>(Random::Int(static_cast<int>(cells.size())))];
            tengu->setState(Mob::State::Passive);
            addMob(std::move(tengu));
        }
        return;
    }
    if(depth_==15||depth_==20||depth_==25)return;
    if(depth_==5){
        std::vector<int>cells;
        for(int cell=0;cell<Length;++cell)if(cell/Width<18&&passable(cell)&&cell!=exit()&&!mobAt(cell))cells.push_back(cell);
        if(cells.empty())return;
        std::unique_ptr<Mob>goo=Bestiary::mutableMob(depth_);if(goo){goo->pos=cells[static_cast<std::size_t>(Random::Int(static_cast<int>(cells.size())))];addMob(std::move(goo));}
        std::unique_ptr<DungeonNPC>king(new DungeonNPC(NPCKind::RatKing));
        king->pos=7+22*Width;
        addMob(std::move(king));
        return;
    }
    if(depth_==21){
        if(dungeonContext()&&dungeonContext()->questState(3)==2){
            for(const auto& room:rooms_){
                if(!room||room->type!=Room::Type::Shop)continue;
                std::unique_ptr<DungeonNPC>npc(new DungeonNPC(NPCKind::ImpShopkeeper));
                npc->pos=room->center().y*Width+room->center().x;
                const int shopkeeperPos=npc->pos;
                addMob(std::move(npc));
                for(const int offset:Neighbours9){
                    const int cell=shopkeeperPos+offset;
                    if(inBoundsCell(cell)&&this->cell(cell)==Terrain::EMPTY_SP)setRaw(cell,Terrain::WATER);
                }
                buildFlagMaps();
                break;
            }
        }
        return;
    }
    const int count = nMobs();

    for (int i = 0; i < count; ++i) {
        std::unique_ptr<Mob> mob =
            Bestiary::mutableMob(depth_);

        if (!mob) {
            continue;
        }

        int pos = -1;

        if (bossLevel(depth_) && roomExit_) {
            const Point point = roomExit_->random(1);
            pos = point.y * Width + point.x;
        }

        while (pos == -1) {
            pos = randomRespawnCell();
        }

        mob->pos = pos;
        addMob(std::move(mob));
    }
    NPCKind npcKind;
    bool addNpc=false;
    int questBit=-1;
    if(Dungeon*game=dungeonContext()){
        if(depth_>1&&depth_<5&&!game->questNpcSpawned(0)&&Random::Int(5-depth_)==0){
            npcKind=NPCKind::Ghost;
            addNpc=true;
            questBit=0;
        }else if(depth_>6&&depth_<10&&!game->questNpcSpawned(1)&&Random::Int(10-depth_)==0){
            npcKind=NPCKind::Wandmaker;
            addNpc=true;
            questBit=1;
        }else if(depth_>16&&depth_<20&&!game->questNpcSpawned(2)&&Random::Int(20-depth_)==0){
            npcKind=NPCKind::Imp;
            addNpc=true;
            questBit=2;
        }
    }
    if(depth_==6||depth_==11||depth_==16){
        npcKind=NPCKind::Shopkeeper;
        addNpc=true;
    }
    if(addNpc){
        std::unique_ptr<DungeonNPC>npc(new DungeonNPC(npcKind));
        npc->pos=-1;
        if(npcKind==NPCKind::Shopkeeper){
            for(const auto&room:rooms_)if(room&&room->type==Room::Type::Shop){
                npc->pos=room->center().y*Width+room->center().x;
                break;
            }
        }else if(npcKind==NPCKind::Wandmaker&&roomEntrance_){
            // The wandmaker uses the entrance room, but never occupies its stairs or sign.
            for(int attempt=0;attempt<30;++attempt){
                const Point point=roomEntrance_->random();
                const int cell=point.y*Width+point.x;
                if(this->cell(cell)!=Terrain::ENTRANCE&&this->cell(cell)!=Terrain::SIGN&&!mobAt(cell)){
                    npc->pos=cell;
                    break;
                }
            }
            if(npc->pos<0)npc->pos=randomRespawnCell();
        }else if(npcKind==NPCKind::Imp){
            // Imp.Quest.spawn rejects occupied heap cells in addition to the normal respawn rules.
            for(int attempt=0;attempt<30;++attempt){
                const int cell=randomRespawnCell();
                if(cell>=0&&!heapAt(cell)){
                    npc->pos=cell;
                    break;
                }
            }
        }else{
            npc->pos=randomRespawnCell();
        }
        if(npc->pos>=0){
            addMob(std::move(npc));
            if(questBit>=0)if(Dungeon*game=dungeonContext()){
                game->markQuestNpcSpawned(questBit);
                // The original chooses the sad ghost's request when it spawns,
                // before the player has accepted it.
                // Keep the ghost's request non-hostile.  The old third branch
                // replaced the NPC with an unrendered curse mob, which looked
                // like a rat attacking immediately after the conversation.
                if(npcKind==NPCKind::Ghost)game->setQuestState(0,701+Random::Int(2));
                if(npcKind==NPCKind::Wandmaker)game->setQuestState(1,711+Random::Int(3));
                // Likewise the imp's monk/golem branch is fixed at spawn time.
                if(npcKind==NPCKind::Imp)game->setQuestState(3,Random::Int(2)==0?401:402);
            }
        }
    }
}

}
