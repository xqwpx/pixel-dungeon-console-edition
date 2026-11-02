#include "game/actors/blobs/Alchemy.h"
#include "game/Dungeon.h"
#include "game/items/Heap.h"
#include "game/items/ConsumableCatalog.h"
#include "game/items/Generator.h"
#include "game/items/potions/PotionOfHealing.h"
#include "game/items/potions/PotionOfToxicGas.h"
#include "game/items/potions/PotionOfParalyticGas.h"
#include "game/items/potions/PotionOfLiquidFlame.h"
#include "game/items/potions/PotionOfMindVision.h"
#include "game/items/potions/PotionOfInvisibility.h"
#include "game/items/potions/PotionOfFrost.h"
#include "game/items/potions/PotionOfStrength.h"
#include "game/Progression.h"
#include "game/utils/Random.h"
namespace pdv { namespace {std::unique_ptr<Item> potionFor(SeedKind k){switch(k){case SeedKind::Firebloom:return std::unique_ptr<Item>(new PotionOfLiquidFlame());case SeedKind::Icecap:return std::unique_ptr<Item>(new PotionOfFrost());case SeedKind::Sorrowmoss:return std::unique_ptr<Item>(new PotionOfToxicGas());case SeedKind::Dreamweed:return std::unique_ptr<Item>(new PotionOfInvisibility());case SeedKind::Sungrass:return std::unique_ptr<Item>(new PotionOfHealing());case SeedKind::Earthroot:return std::unique_ptr<Item>(new PotionOfParalyticGas());case SeedKind::Fadeleaf:return std::unique_ptr<Item>(new PotionOfMindVision());case SeedKind::Rotberry:return std::unique_ptr<Item>(new PotionOfStrength());}return nullptr;}}
void Alchemy::seed(int cell,int amount){if(pos_>=0)clear(pos_);pos_=cell;Blob::seed(cell,amount);}void Alchemy::evolve(){if(pos_<0)return;off_[static_cast<std::size_t>(pos_)]=cur_[static_cast<std::size_t>(pos_)];volume_=off_[static_cast<std::size_t>(pos_)];if(dungeon()&&dungeon()->visible()[static_cast<std::size_t>(pos_)])Progression::instance().addJournal(JournalFeature::Alchemy,dungeon()->depth());}
bool Alchemy::transmute(int cell){if(!dungeon()||cell!=pos_)return false;Heap*heap=dungeon()->level().heapAt(cell);if(!heap)return false;int total=0;std::vector<SeedKind> weighted;for(const auto&i:heap->items()){const CatalogSeed*s=dynamic_cast<const CatalogSeed*>(i.get());if(!s)return false;for(int n=0;n<s->quantity();++n)weighted.push_back(s->kind());total+=s->quantity();}if(total<3)return false;std::unique_ptr<Item> result=Random::Int(total)==0?Generator::random(Generator::Category::Potion,dungeon()->depth()):potionFor(weighted[static_cast<std::size_t>(Random::Int(static_cast<int>(weighted.size())))]);while(!heap->empty())heap->pickUp();dungeon()->level().removeEmptyHeap(cell);dungeon()->level().drop(std::move(result),cell);Progression::instance().notePotionCooked();return true;}}
