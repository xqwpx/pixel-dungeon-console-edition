#include "game/items/potions/PotionOfFrost.h"
#include "game/Dungeon.h"
#include "game/actors/Char.h"
#include "game/actors/blobs/Blob.h"
#include "game/actors/buffs/Buff.h"
#include "game/actors/buffs/Frost.h"
#include "game/items/Heap.h"
#include "game/items/ItemSpriteSheet.h"
#include "game/pathfinding/PathFinder.h"
#include "game/utils/Random.h"
#include <array>
namespace pdv {
PotionOfFrost::PotionOfFrost(){name_="Potion of Frost";image_=ItemSpriteSheet::POTION_SILVER;}
void PotionOfFrost::shatter(Dungeon&d,int cell){
 std::array<bool,Level::Length> pass{}; for(int i=0;i<Level::Length;++i)pass[i]=!d.level().losBlocking(i);
 PathFinder::buildDistanceMap(cell,pass,Distance); const auto&dist=PathFinder::distance(); bool visible=false;
 Blob* fire=d.level().blob(BlobKind::Fire);
 for(int i=0;i<Level::Length;++i){
  if(i>=static_cast<int>(dist.size())||dist[i]>Distance)continue;
  if(fire&&fire->valueAt(i)>0)fire->clear(i);
  if(Char*ch=d.scheduler().findChar(i))Buff::prolong<Frost>(*ch,Frost::duration(*ch)*Random::Float(1.0f,1.5f));
  if(Heap*heap=d.level().heapAt(i)){
   if(heap->type==Heap::Type::Mimic)d.activateMimicAt(i,false);
   else heap->freeze();
  }
  if(d.visible()[static_cast<std::size_t>(i)])visible=true;
 }
 if(visible)setKnown();
}
std::unique_ptr<Item> PotionOfFrost::clone() const{return std::unique_ptr<Item>(new PotionOfFrost(*this));}
}
