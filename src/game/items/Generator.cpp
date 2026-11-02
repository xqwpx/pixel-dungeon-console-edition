#include "game/items/Generator.h"
#include "game/items/Gold.h"
#include "game/items/food/Food.h"
#include "game/items/food/Pasty.h"
#include "game/items/potions/PotionOfHealing.h"
#include "game/items/potions/PotionOfExperience.h"
#include "game/items/potions/PotionOfToxicGas.h"
#include "game/items/potions/PotionOfParalyticGas.h"
#include "game/items/potions/PotionOfLiquidFlame.h"
#include "game/items/potions/PotionOfLevitation.h"
#include "game/items/potions/PotionOfStrength.h"
#include "game/items/potions/PotionOfMindVision.h"
#include "game/items/potions/PotionOfPurity.h"
#include "game/items/potions/PotionOfInvisibility.h"
#include "game/items/potions/PotionOfMight.h"
#include "game/items/potions/PotionOfFrost.h"
#include "game/items/scrolls/Scroll.h"
#include "game/items/seeds/Seed.h"
#include "game/items/EquipmentCatalog.h"
#include "game/items/ConsumableCatalog.h"
#include "game/items/armor/ClothArmor.h"
#include "game/items/rings/RingOfShadows.h"
#include "game/items/wands/WandOfMagicMissile.h"
#include "game/items/weapon/melee/Dagger.h"
#include "game/items/weapon/melee/Knuckles.h"
#include "game/items/weapon/melee/ShortSword.h"
#include "game/items/weapon/missiles/Boomerang.h"
#include "game/utils/Random.h"
#include <algorithm>
namespace pdv {
namespace {
using C=Generator::Category;
static const std::array<float, static_cast<std::size_t>(C::Count)> kBase = {{15,10,50,40,4,2,5,0,50,5}};
static const std::array<float,12> kPotion = {{45,4,15,10,15,10,0,20,12,10,0,10}};
static const std::array<float,12> kScroll = {{30,10,15,10,15,12,8,8,4,6,0,1}};
static const std::array<float,3> kFood = {{4,1,0}};
static const float kSeed[8]={1,1,1,1,1,1,1,0};
static const float kMisc[2]={2,1};
static const float kWeapon[18]={1,1,1,1,1,1,1,1,1,1,0,0,1,1,1,1,0,1};
static const float kWand[13]={10,10,15,6,10,11,15,10,6,10,0,5,5};
int weightedIndex(const float* probs, int n) {
    float total=0; for(int i=0;i<n;++i) total += std::max(0.0f, probs[i]);
    if(total<=0) return 0;
    float r=Random::Float(total);
    for(int i=0;i<n;++i){ r -= std::max(0.0f,probs[i]); if(r<0) return i; }
    return n-1;
}
std::unique_ptr<Item> makePotion(int i) {
    switch(i){
        case 0:return std::unique_ptr<Item>(new PotionOfHealing());
        case 1:return std::unique_ptr<Item>(new PotionOfExperience());
        case 2:return std::unique_ptr<Item>(new PotionOfToxicGas());
        case 3:return std::unique_ptr<Item>(new PotionOfParalyticGas());
        case 4:return std::unique_ptr<Item>(new PotionOfLiquidFlame());
        case 5:return std::unique_ptr<Item>(new PotionOfLevitation());
        case 6:return std::unique_ptr<Item>(new PotionOfStrength());
        case 7:return std::unique_ptr<Item>(new PotionOfMindVision());
        case 8:return std::unique_ptr<Item>(new PotionOfPurity());
        case 9:return std::unique_ptr<Item>(new PotionOfInvisibility());
        case 10:return std::unique_ptr<Item>(new PotionOfMight());
        case 11:return std::unique_ptr<Item>(new PotionOfFrost());
    }
    return nullptr;
}
std::unique_ptr<Item> makeScroll(int i) {
    static const ScrollKind kinds[12] = {
        ScrollKind::Identify, ScrollKind::Teleportation, ScrollKind::RemoveCurse,
        ScrollKind::Recharging, ScrollKind::MagicMapping, ScrollKind::Challenge,
        ScrollKind::Terror, ScrollKind::Lullaby, ScrollKind::PsionicBlast,
        ScrollKind::MirrorImage, ScrollKind::Upgrade, ScrollKind::Enchantment
    };
    if(i==10) return std::unique_ptr<Item>(new ScrollOfUpgrade());
    if(i==11) return std::unique_ptr<Item>(new ScrollOfEnchantment());
    return std::unique_ptr<Item>(new Scroll(kinds[std::max(0,std::min(i,11))]));
}
std::unique_ptr<Item> makeWeapon(int i){switch(i){case 0:return std::unique_ptr<Item>(new Dagger());case 1:return std::unique_ptr<Item>(new Knuckles());case 2:return std::unique_ptr<Item>(new CatalogMelee(MeleeKind::Quarterstaff));case 3:return std::unique_ptr<Item>(new CatalogMelee(MeleeKind::Spear));case 4:return std::unique_ptr<Item>(new CatalogMelee(MeleeKind::Mace));case 5:return std::unique_ptr<Item>(new CatalogMelee(MeleeKind::Sword));case 6:return std::unique_ptr<Item>(new CatalogMelee(MeleeKind::Longsword));case 7:return std::unique_ptr<Item>(new CatalogMelee(MeleeKind::BattleAxe));case 8:return std::unique_ptr<Item>(new CatalogMelee(MeleeKind::WarHammer));case 9:return std::unique_ptr<Item>(new CatalogMelee(MeleeKind::Glaive));case 10:return std::unique_ptr<Item>(new ShortSword());case 12:return std::unique_ptr<Item>(new CatalogMissile(MissileKind::Javelin));case 13:return std::unique_ptr<Item>(new CatalogMissile(MissileKind::IncendiaryDart));case 14:return std::unique_ptr<Item>(new CatalogMissile(MissileKind::CurareDart));case 15:return std::unique_ptr<Item>(new CatalogMissile(MissileKind::Shuriken));case 16:return std::unique_ptr<Item>(new Boomerang());default:return std::unique_ptr<Item>(new CatalogMissile(MissileKind::Tomahawk));}}
std::unique_ptr<Item> makeArmor(int i){if(i==0)return std::unique_ptr<Item>(new ClothArmor());return std::unique_ptr<Item>(new CatalogArmor(static_cast<ArmorKind>(i+1)));}
std::unique_ptr<Item> makeWand(int i){if(i==10)return std::unique_ptr<Item>(new WandOfMagicMissile());const int kind=i<10?i:i-1;return std::unique_ptr<Item>(new CatalogWand(static_cast<WandKind>(kind)));}
std::unique_ptr<Item> makeRing(int i){if(i==2)return std::unique_ptr<Item>(new RingOfShadows());return std::unique_ptr<Item>(new CatalogRing(static_cast<RingKind>(i<2?i:i-1)));}
std::unique_ptr<Item> randomEquipment(std::unique_ptr<Item>item,C category){
    if(!item)return item;
    if(category==C::Weapon)if(CatalogMissile*missile=dynamic_cast<CatalogMissile*>(item.get())){switch(missile->kind()){case MissileKind::Javelin:case MissileKind::Shuriken:missile->quantity(Random::Int(5,15));break;case MissileKind::IncendiaryDart:missile->quantity(Random::Int(3,6));break;case MissileKind::CurareDart:missile->quantity(Random::Int(2,5));break;case MissileKind::Tomahawk:missile->quantity(Random::Int(5,12));break;}return item;}
    if(category==C::Weapon&&Random::Float()<0.4f){int n=1;if(Random::Int(3)==0){++n;if(Random::Int(3)==0)++n;}if(Random::Int(2)==0)item->upgrade(n);else{item->degrade(n);item->setCursed(true);}}
    else if(category==C::Armor){if(Random::Float()<0.4f){int n=1;if(Random::Int(3)==0){++n;if(Random::Int(3)==0)++n;}if(Random::Int(2)==0)item->upgrade(n);else{item->degrade(n);item->setCursed(true);}}if(Random::Int(10)==0)if(Armor*armor=dynamic_cast<Armor*>(item.get()))armor->inscribe();}
    else if(category==C::Wand){if(Random::Float()<0.5f){item->upgrade();if(Random::Float()<0.15f)item->upgrade();}}
    else if(category==C::Ring){CatalogRing*catalog=dynamic_cast<CatalogRing*>(item.get());if(catalog&&(catalog->kind()==RingKind::Haggler||catalog->kind()==RingKind::Thorns))item->upgrade();else{const int n=Random::Int(1,3);if(Random::Float()<0.3f){item->degrade(n);item->setCursed(true);}else item->upgrade(n);}}
    return item;
}
}
std::array<float,static_cast<std::size_t>(Generator::Category::Count)> Generator::categoryProbs_ = kBase;
void Generator::reset(){ categoryProbs_=kBase; }
float Generator::baseProbability(Category c){return kBase[static_cast<std::size_t>(c)];}
float Generator::currentProbability(Category c){return categoryProbs_[static_cast<std::size_t>(c)];}
const std::array<float,12>& Generator::potionProbabilities(){return kPotion;}
const std::array<float,12>& Generator::scrollProbabilities(){return kScroll;}
bool Generator::categoryImplemented(Category){return true;}
Generator::Category Generator::randomCategory(){
    return static_cast<Category>(weightedIndex(categoryProbs_.data(),static_cast<int>(Category::Count)));
}
std::unique_ptr<Item> Generator::random(int depth){ return random(randomCategory(),depth); }
std::unique_ptr<Item> Generator::random(Category c,int depth){
    auto& p=categoryProbs_[static_cast<std::size_t>(c)]; p *= 0.5f;
    switch(c){
        case Category::Gold:{ auto g=std::unique_ptr<Gold>(new Gold()); g->random(depth); return std::unique_ptr<Item>(g.release()); }
        case Category::Food:{const int i=weightedIndex(kFood.data(),3);return i==1?std::unique_ptr<Item>(new Pasty()):(i==2?std::unique_ptr<Item>(new MysteryMeat()):std::unique_ptr<Item>(new Food()));}
        case Category::Potion:return makePotion(weightedIndex(kPotion.data(),12));
        case Category::Scroll:return makeScroll(weightedIndex(kScroll.data(),12));
        case Category::Weapon:return randomEquipment(makeWeapon(weightedIndex(kWeapon,18)),c);
        case Category::Armor:return randomEquipment(makeArmor(Random::Int(5)),c);
        case Category::Wand:return randomEquipment(makeWand(weightedIndex(kWand,13)),c);
        case Category::Ring:return randomEquipment(makeRing(Random::Int(12)),c);
        case Category::Seed:return std::unique_ptr<Item>(new CatalogSeed(static_cast<SeedKind>(weightedIndex(kSeed,8))));
        case Category::Misc:return std::unique_ptr<Item>(new CatalogMisc(static_cast<MiscKind>(weightedIndex(kMisc,2))));
        default:return nullptr;
    }
}
std::vector<std::unique_ptr<Item>> Generator::allImplementedItems(int depth){
    std::vector<std::unique_ptr<Item>> items;
    for(int i=0;i<18;++i) if(i!=11) items.push_back(makeWeapon(i));
    for(int i=0;i<5;++i) items.push_back(makeArmor(i));
    for(int i=0;i<12;++i) items.push_back(makePotion(i));
    for(int i=0;i<12;++i) items.push_back(makeScroll(i));
    for(int i=0;i<13;++i) items.push_back(makeWand(i));
    for(int i=0;i<12;++i) items.push_back(makeRing(i));
    for(int i=0;i<8;++i) items.push_back(std::unique_ptr<Item>(new CatalogSeed(static_cast<SeedKind>(i))));
    items.push_back(std::unique_ptr<Item>(new Food()));
    items.push_back(std::unique_ptr<Item>(new Pasty()));
    items.push_back(std::unique_ptr<Item>(new MysteryMeat()));
    items.push_back(std::unique_ptr<Item>(new CatalogMisc(MiscKind::Bomb)));
    items.push_back(std::unique_ptr<Item>(new CatalogMisc(MiscKind::Honeypot)));
    auto gold=std::unique_ptr<Gold>(new Gold()); gold->random(std::max(1,depth)); items.push_back(std::move(gold));
    return items;
}
}
