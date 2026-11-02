#include "game/actors/mobs/npcs/DungeonNPC.h"
#include "game/Dungeon.h"
#include "game/actors/Hero.h"
#include "game/items/EquipmentCatalog.h"
#include "game/items/ConsumableCatalog.h"
#include "game/items/Generator.h"
#include "game/items/Heap.h"
#include "game/items/SpecialItems.h"
#include "game/items/rings/RingOfShadows.h"
#include "game/levels/Terrain.h"
#include "game/utils/Random.h"
#include "game/Progression.h"
#include <algorithm>
#include <vector>
namespace pdv {
DungeonNPC::DungeonNPC(NPCKind kind):kind_(kind){hostile_=false;state_=State::Passive;defenseSkill_=1000;HP=HT=1;exp_=0;flying=kind==NPCKind::Ghost;static const char*n[]={"sad ghost","old wandmaker","troll blacksmith","ambitious imp","shopkeeper","rat king","imp shopkeeper"};name=n[static_cast<int>(kind)];}
bool DungeonNPC::act(){spend(Actor::TICK);next();return true;}
void DungeonNPC::die(const void*cause){
    corpseVisible_=false;
    if(cause&&dungeon_){
        Heap* remains=dungeon_->level().drop(std::unique_ptr<Item>(new SpecialItem(SpecialKind::NPCBones)),pos);
        if(remains)remains->type=Heap::Type::Skeleton;
        dungeon_->postMessage("Killed "+name);
    }
    Mob::die(cause);
}
namespace {SpecialItem* questItem(Hero&hero,SpecialKind kind){for(Item*item:hero.belongings().allItems()){SpecialItem*s=dynamic_cast<SpecialItem*>(item);if(s&&s->kind()==kind)return s;}return nullptr;}}
namespace {CatalogSeed* questSeed(Hero&hero,SeedKind kind){for(Item*item:hero.belongings().allItems()){CatalogSeed*seed=dynamic_cast<CatalogSeed*>(item);if(seed&&seed->kind()==kind)return seed;}return nullptr;}}
namespace {QuestPickaxe* questPickaxe(Hero&hero){for(Item*item:hero.belongings().allItems())if(QuestPickaxe*pick=dynamic_cast<QuestPickaxe*>(item))return pick;return nullptr;}}
namespace {int emptyQuestCell(Dungeon&d){for(int attempt=0;attempt<100;++attempt){const int cell=Random::Int(Level::Length);if(d.level().passable(cell)&&!d.level().heapAt(cell)&&!d.scheduler().findChar(cell))return cell;}return -1;}}
void DungeonNPC::interact(Dungeon&d,Hero&hero){
 if(kind_==NPCKind::Ghost)Progression::instance().addJournal(JournalFeature::Ghost,d.depth());else if(kind_==NPCKind::Wandmaker)Progression::instance().addJournal(JournalFeature::Wandmaker,d.depth());else if(kind_==NPCKind::Blacksmith)Progression::instance().addJournal(JournalFeature::Troll,d.depth());else if(kind_==NPCKind::Imp)Progression::instance().addJournal(JournalFeature::Imp,d.depth());
 switch(kind_){
 case NPCKind::Ghost:{const int state=d.questState(0);SpecialItem*rose=questItem(hero,SpecialKind::DriedRose);SpecialItem*skull=questItem(hero,SpecialKind::RatSkull);const bool roseDone=state==150&&rose;const bool ratDone=(state==200||state==201)&&skull;if(roseDone||ratDone){if(roseDone)hero.belongings().detach(rose);if(ratDone)hero.belongings().detach(skull);d.setQuestState(0,360);d.postMessage("The ghost offers you a choice before finding peace.");die(nullptr);}else if(state==0||state==701||state==702||state==703){const int type=state==702?1:0;if(type==0){d.setQuestState(0,108);d.postMessage("Ghost: defeat creatures until you recover my dried rose.");}else{d.setQuestState(0,200);d.postMessage("Ghost: slay the fetid rat and bring me its skull.");}}else if(state>=101&&state<=108)d.postMessage("Ghost: the rose is still carried by one of this floor's creatures.");else if(state==150)d.postMessage("Ghost: find the dried rose that fell nearby.");else if(state==200||state==201)d.postMessage("Ghost: the fetid rat still lives.");else {d.setQuestState(0,701);d.postMessage("Ghost: defeat creatures until you recover my dried rose.");}break;}
 case NPCKind::Wandmaker:{int state=d.questState(1);CatalogSeed*berry=questSeed(hero,SeedKind::Rotberry);SpecialItem*dust=questItem(hero,SpecialKind::CorpseDust);SpecialItem*fish=questItem(hero,SpecialKind::PhantomFish);Item*ingredient=state==501?static_cast<Item*>(berry):(state==502?static_cast<Item*>(dust):(state==503?static_cast<Item*>(fish):nullptr));if(ingredient){hero.belongings().detach(ingredient);static const WandKind first[]={WandKind::Avalanche,WandKind::Disintegration,WandKind::Firebolt,WandKind::Lightning,WandKind::Poison};static const WandKind second[]={WandKind::Amok,WandKind::Blink,WandKind::Regrowth,WandKind::Slowness,WandKind::Reach};d.setQuestState(1,1000+static_cast<int>(first[Random::Int(5)])*16+static_cast<int>(second[Random::Int(5)]));d.postMessage("Wandmaker: choose one of my two finest wands.");}else if(state==0||(state>=711&&state<786)){const int packed=state>=711?(state-711):Random::Int(75);const int type=packed%3;if(type==0){state=501;const int cell=emptyQuestCell(d);if(cell>=0)d.level().plant(SeedKind::Rotberry,cell);d.postMessage("Wandmaker: bring me a Rotberry seed.");}else if(type==1){state=502;std::vector<int>candidates;for(const auto&entry:d.level().heaps())if(entry.second&&entry.second->type==Heap::Type::Skeleton&&!d.visible()[static_cast<std::size_t>(entry.first)])candidates.push_back(entry.first);const int cell=candidates.empty()?emptyQuestCell(d):candidates[static_cast<std::size_t>(Random::Int(static_cast<int>(candidates.size())))];Heap*heap=cell>=0?d.level().drop(std::unique_ptr<Item>(new SpecialItem(SpecialKind::CorpseDust)),cell):nullptr;if(heap&&candidates.empty())heap->type=Heap::Type::Skeleton;d.postMessage("Wandmaker: bring me corpse dust from skeletal remains.");}else{state=503;int cell=-1;for(int i=0;i<100;++i){const int p=Random::Int(Level::Length);if(d.level().water(p)){cell=p;break;}}if(cell<0)cell=emptyQuestCell(d);Heap*heap=cell>=0?d.level().drop(std::unique_ptr<Item>(new SpecialItem(SpecialKind::PhantomFish)),cell):nullptr;if(heap&&d.level().water(cell))heap->type=Heap::Type::Hidden;d.postMessage("Wandmaker: find a phantom fish hidden in the water.");}d.setQuestState(1,state);}else if(state==2)d.postMessage("Wandmaker: good luck on your journey.");else d.postMessage(state==501?"Wandmaker: the Rotberry shrub is somewhere on this floor.":state==502?"Wandmaker: search the skeletal remains for corpse dust.":"Wandmaker: search the water for the phantom fish.");break;}
 case NPCKind::Blacksmith:{int state=d.questState(2);QuestPickaxe*pick=questPickaxe(hero);SpecialItem*ore=questItem(hero,SpecialKind::DarkGold);if(state==0){const bool blood=d.blacksmithQuestKind()==2||(d.blacksmithQuestKind()==0&&Random::Int(2)==0);if(d.blacksmithQuestKind()==0)d.setBlacksmithQuestKind(blood?2:1);std::unique_ptr<QuestPickaxe>given(new QuestPickaxe());if(!hero.belongings().backpack().collect(std::move(given)))d.level().drop(std::unique_ptr<Item>(new QuestPickaxe()),hero.pos);if(!blood){std::vector<int> veins;for(int cell=0;cell<Level::Length;++cell)if(d.level().cell(cell)==Terrain::WALL)veins.push_back(cell);const int count=std::min(15,static_cast<int>(veins.size()));for(int i=0;i<count;++i){const int index=Random::Int(static_cast<int>(veins.size()));d.level().setRaw(veins[static_cast<std::size_t>(index)],Terrain::WALL_DECO);veins.erase(veins.begin()+index);}d.level().buildFlagMaps();}d.setQuestState(2,blood?602:601);d.postMessage(blood?"Blacksmith: kill a bat with this pickaxe and bring it back blood-stained.":"Blacksmith: mine 15 pieces of dark gold with this pickaxe.");}else if(state==601||state==602){const bool ready=pick&&((state==601&&ore&&ore->quantity()>=15)||(state==602&&pick->bloodStained()));if(!pick)d.postMessage("Blacksmith: where is my pickaxe?");else if(!ready)d.postMessage(state==601?"Blacksmith: I still need 15 pieces of dark gold.":"Blacksmith: I need bat blood on the pickaxe.");else{if(hero.belongings().weapon()==pick)hero.belongings().takeWeapon();else hero.belongings().backpack().removeAll(pick);if(state==601)hero.belongings().backpack().removeAll(ore);d.setQuestState(2,650);d.postMessage("Blacksmith: select two matching items to reforge.");}}else if(state==650)d.postMessage("Blacksmith: select two matching items to reforge.");else d.postMessage("Blacksmith: I'm busy. Get lost!");break;}
 case NPCKind::Imp:{SpecialItem*token=questItem(hero,SpecialKind::DwarfToken);int state=d.questState(3);if(state==0){state=Random::Int(2)==0?401:402;d.setQuestState(3,state);}if(state==2)d.postMessage("Imp: our deal is already complete.");else{const int needed=state==401?8:6;if(token&&token->quantity()>=needed){token->quantity(token->quantity()-needed);if(token->quantity()<=0)hero.belongings().detach(token);const int first=Random::Int(11);int second=Random::Int(10);if(second>=first)++second;d.setQuestState(3,2000+first*16+second);d.postMessage("Imp: choose one of my two reward rings.");}else d.postMessage(state==401?"Imp: bring me 8 tokens from dwarf monks.":"Imp: bring me 6 tokens from golems.");}break;}
 case NPCKind::Shopkeeper:d.postMessage("Shopkeeper: items on the floor are for sale. Step on one to buy it.");break;
 case NPCKind::ImpShopkeeper:d.postMessage("Imp: my finest wares are on the floor. Step on one to buy it.");break;
 case NPCKind::RatKing:d.addGold(100);d.postMessage("Rat King: tribute accepted. Take 100 gold!");die(nullptr);break;
 }
 if(kind_==NPCKind::Ghost&&d.questState(0)==2)Progression::instance().removeJournal(JournalFeature::Ghost,d.depth());if(kind_==NPCKind::Wandmaker&&d.questState(1)==2)Progression::instance().removeJournal(JournalFeature::Wandmaker,d.depth());if(kind_==NPCKind::Blacksmith&&d.questState(2)==2)Progression::instance().removeJournal(JournalFeature::Troll,d.depth());if(kind_==NPCKind::Imp&&d.questState(3)==2)Progression::instance().removeJournal(JournalFeature::Imp,d.depth());
}
const char*DungeonNPC::description()const{switch(kind_){
case NPCKind::Ghost:return "A sorrowful spirit, barely visible in the dungeon gloom.";
case NPCKind::Wandmaker:return "An old wandmaker protected by a magical shield.";
case NPCKind::Blacksmith:return "A troll blacksmith who knows how to reforge worthy equipment.";
case NPCKind::Imp:return "A clever imp looking for someone to do his dangerous work.";
case NPCKind::Shopkeeper:return "A shopkeeper guarding the wares spread around the room.";
case NPCKind::RatKing:return "The self-proclaimed king of the dungeon rats.";
case NPCKind::ImpShopkeeper:return "An imp merchant offering his finest wares.";
}return "A peaceful dungeon inhabitant.";}
}
