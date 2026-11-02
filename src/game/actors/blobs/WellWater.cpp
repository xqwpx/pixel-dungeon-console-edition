#include "game/actors/blobs/WellWater.h"
#include "game/Dungeon.h"
#include "game/actors/Hero.h"
#include "game/actors/buffs/Awareness.h"
#include "game/actors/buffs/Hunger.h"
#include "game/items/Heap.h"
#include "game/items/SpecialItems.h"
#include "game/items/Generator.h"
#include "game/items/EquipmentCatalog.h"
#include "game/items/potions/Potion.h"
#include "game/items/potions/PotionOfStrength.h"
#include "game/items/potions/PotionOfMight.h"
#include "game/items/scrolls/Scroll.h"
#include "game/items/wands/Wand.h"
#include "game/items/rings/Ring.h"
#include "game/items/ConsumableCatalog.h"
#include "game/items/weapon/melee/Dagger.h"
#include "game/items/weapon/melee/Knuckles.h"
#include "game/levels/Terrain.h"
#include "game/utils/Random.h"
#include "game/Progression.h"
#include <algorithm>
#include <vector>

namespace pdv {
namespace {
std::unique_ptr<Item> transmute(const Item& old,int depth){
 std::unique_ptr<Item> out;
 if(dynamic_cast<const Knuckles*>(&old))out.reset(new Dagger());else if(dynamic_cast<const Dagger*>(&old))out.reset(new Knuckles());
 else if(const CatalogMelee*w=dynamic_cast<const CatalogMelee*>(&old)){MeleeKind k=w->kind();switch(k){case MeleeKind::Quarterstaff:k=MeleeKind::Spear;break;case MeleeKind::Spear:k=MeleeKind::Quarterstaff;break;case MeleeKind::Mace:k=MeleeKind::Sword;break;case MeleeKind::Sword:k=MeleeKind::Mace;break;case MeleeKind::Longsword:k=MeleeKind::BattleAxe;break;case MeleeKind::BattleAxe:k=MeleeKind::Longsword;break;case MeleeKind::WarHammer:k=MeleeKind::Glaive;break;case MeleeKind::Glaive:k=MeleeKind::WarHammer;break;}out.reset(new CatalogMelee(k));}
 else if(dynamic_cast<const ScrollOfUpgrade*>(&old))out.reset(new ScrollOfEnchantment());else if(dynamic_cast<const ScrollOfEnchantment*>(&old))out.reset(new ScrollOfUpgrade());
 else if(dynamic_cast<const PotionOfStrength*>(&old))out.reset(new PotionOfMight());else if(dynamic_cast<const PotionOfMight*>(&old))out.reset(new PotionOfStrength());
 else if(const CatalogRing*r=dynamic_cast<const CatalogRing*>(&old)){RingKind k;do{k=static_cast<RingKind>(Random::Int(11));}while(k==r->kind());out.reset(new CatalogRing(k));}
 else if(const CatalogWand*w=dynamic_cast<const CatalogWand*>(&old)){WandKind k;do{k=static_cast<WandKind>(Random::Int(12));}while(k==w->kind());out.reset(new CatalogWand(k));}
 else if(const CatalogSeed*s=dynamic_cast<const CatalogSeed*>(&old)){SeedKind k;do{k=static_cast<SeedKind>(Random::Int(8));}while(k==s->kind());out.reset(new CatalogSeed(k));}
 else if(dynamic_cast<const Scroll*>(&old)){do{out=Generator::random(Generator::Category::Scroll,depth);}while(out&&out->sameKind(old));}
 else if(dynamic_cast<const Potion*>(&old)){do{out=Generator::random(Generator::Category::Potion,depth);}while(out&&out->sameKind(old));}
 else if(dynamic_cast<const Wand*>(&old)){do{out=Generator::random(Generator::Category::Wand,depth);}while(out&&out->sameKind(old));}
 else if(dynamic_cast<const Ring*>(&old)){do{out=Generator::random(Generator::Category::Ring,depth);}while(out&&out->sameKind(old));}
 if(out){
     const bool preserveState=dynamic_cast<const MeleeWeapon*>(&old)||dynamic_cast<const Ring*>(&old)||dynamic_cast<const Wand*>(&old);
     if(preserveState){
         out->level(old.level());
         out->setCursed(old.cursed());
         out->setLevelKnown(old.levelKnown());
         out->setCursedKnown(old.cursedKnown());
         const Weapon* oldWeapon=dynamic_cast<const Weapon*>(&old);
         Weapon* newWeapon=dynamic_cast<Weapon*>(out.get());
         if(oldWeapon&&newWeapon&&oldWeapon->isEnchanted())newWeapon->enchant();
     }
 }
 return out;
}
}
void WellWater::seed(int cell,int amount){if(pos_>=0)clear(pos_);pos_=cell;Blob::seed(cell,amount);}
void WellWater::evolve(){if(pos_<0)return;off_[static_cast<std::size_t>(pos_)]=cur_[static_cast<std::size_t>(pos_)];volume_=off_[static_cast<std::size_t>(pos_)];if(dungeon()&&dungeon()->visible()[static_cast<std::size_t>(pos_)]){const JournalFeature feature=kind_==BlobKind::WaterHealth?JournalFeature::WellHealth:(kind_==BlobKind::WaterAwareness?JournalFeature::WellAwareness:JournalFeature::WellTransmutation);Progression::instance().addJournal(feature,dungeon()->depth());}}
bool WellWater::affect(int cell){if(!dungeon()||cell!=pos_||valueAt(cell)<=0)return false;Dungeon& game=*dungeon();Hero& hero=game.hero();bool used=false;
 if(kind_==BlobKind::WaterHealth&&hero.pos==cell){hero.HP=hero.HT;hero.belongings().uncurseEquipped();game.hunger().satisfy(Hunger::Starving);used=true;}
 else if(kind_==BlobKind::WaterAwareness&&hero.pos==cell){hero.belongings().observeEquipped();for(int i=0;i<Level::Length;++i){const int t=game.level().cell(i);if(Terrain::flagsFor(t)&Terrain::SECRET)game.level().setRaw(i,Terrain::discover(t));}game.level().buildFlagMaps();Buff::affect<Awareness>(hero,Awareness::Duration);used=true;}
else if(Heap* heap=game.level().heapAt(cell)){Item* old=heap->peek();if(kind_==BlobKind::WaterHealth){if(SpecialItem*v=dynamic_cast<SpecialItem*>(old)){if(v->kind()==SpecialKind::DewVial&&v->quantity()<10){v->quantity(10);used=true;}}}else if(kind_==BlobKind::WaterAwareness){if(old&&!old->isIdentified()){old->identify();used=true;}}else if(kind_==BlobKind::WaterTransmutation&&old){std::unique_ptr<Item> changed=transmute(*old,game.depth());if(changed){if(old->quantity()>1){old->quantity(old->quantity()-1);heap->drop(std::move(changed));}else heap->replaceTop(std::move(changed));used=true;}}
 if(!used&&old){std::vector<int> destinations;for(const int offset:Level::Neighbours8){const int destination=cell+offset;if(game.level().inBoundsCell(destination)&&(game.level().passable(destination)||game.level().avoid(destination)))destinations.push_back(destination);}if(!destinations.empty()){std::unique_ptr<Item> displaced=heap->pickUp();game.level().removeEmptyHeap(cell);game.level().drop(std::move(displaced),destinations[static_cast<std::size_t>(Random::Int(static_cast<int>(destinations.size())))]);}}
}
 if(used){clear(cell);game.level().setRaw(cell,Terrain::EMPTY_WELL);game.level().buildFlagMaps();const JournalFeature feature=kind_==BlobKind::WaterHealth?JournalFeature::WellHealth:(kind_==BlobKind::WaterAwareness?JournalFeature::WellAwareness:JournalFeature::WellTransmutation);Progression::instance().removeJournal(feature,game.depth());}return used;}
}
