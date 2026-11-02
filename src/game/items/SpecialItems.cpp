#include "game/items/SpecialItems.h"
#include "game/items/ItemSpriteSheet.h"
#include "game/items/wands/Wand.h"
#include "game/actors/Hero.h"
namespace pdv {
SpecialItem::SpecialItem(SpecialKind kind):kind_(kind){static const char*n[]={"Amulet of Yendor","ankh","armor kit","dewdrop","dew vial","Lloyd's beacon","tome of mastery","torch","weightstone","corpse dust","dark gold ore","dried rose","dwarf token","phantom fish","pickaxe","rat skull","bones","unknown chest contents"};static const int image[]={87,1,86,81,120,85,82,84,123,121,102,100,122,118,101,103,ItemSpriteSheet::BONES,ItemSpriteSheet::SMTH};name_=n[static_cast<int>(kind)];image_=image[static_cast<int>(kind)];stackable_=kind==SpecialKind::Ankh||kind==SpecialKind::Dewdrop||kind==SpecialKind::Torch||kind==SpecialKind::Weightstone||kind==SpecialKind::DarkGold||kind==SpecialKind::DwarfToken;unique_=kind==SpecialKind::Amulet||kind==SpecialKind::ArmorKit||kind==SpecialKind::DewVial||kind==SpecialKind::LloydsBeacon||kind==SpecialKind::TomeOfMastery;if(kind==SpecialKind::DewVial)quantity_=0;}
int SpecialItem::price()const{switch(kind_){case SpecialKind::Ankh:return 50*quantity();case SpecialKind::Weightstone:return 40*quantity();case SpecialKind::Torch:return 10*quantity();default:return Item::price();}}
std::unique_ptr<Item> SpecialItem::clone()const{return std::unique_ptr<Item>(new SpecialItem(*this));}
std::vector<ItemAction> SpecialItem::actions(const Hero& hero)const{
 if(kind_==SpecialKind::TomeOfMastery){switch(hero.heroClass()){case HeroClass::Warrior:return {ItemAction::Gladiator,ItemAction::Berserker,ItemAction::Drop};case HeroClass::Mage:return {ItemAction::Battlemage,ItemAction::Warlock,ItemAction::Drop};case HeroClass::Rogue:return {ItemAction::Assassin,ItemAction::Freerunner,ItemAction::Drop};case HeroClass::Huntress:return {ItemAction::Sniper,ItemAction::Warden,ItemAction::Drop};}}
 switch(kind_){case SpecialKind::Weightstone:return {ItemAction::BalanceSpeed,ItemAction::BalanceAccuracy,ItemAction::Drop};case SpecialKind::Dewdrop:case SpecialKind::DewVial:case SpecialKind::LloydsBeacon:case SpecialKind::Torch:case SpecialKind::ArmorKit:case SpecialKind::Amulet:return {ItemAction::Use,ItemAction::Drop};default:return {ItemAction::Drop};}}
QuestPickaxe::QuestPickaxe(){name_="pickaxe";image_=ItemSpriteSheet::PICKAXE;strength_=14;unique_=true;identify();}
std::unique_ptr<Item>QuestPickaxe::clone()const{return std::unique_ptr<Item>(new QuestPickaxe(*this));}
std::vector<ItemAction>QuestPickaxe::actions(const Hero&hero)const{std::vector<ItemAction>result=Weapon::actions(hero);result.insert(result.begin(),ItemAction::Mine);return result;}
ClassArmor::ClassArmor(ClassArmorKind kind):Armor(6),kind_(kind){static const char*n[]={"warrior armor","mage armor","rogue armor","huntress armor"};static const int image[]={97,98,96,99};name_=n[static_cast<int>(kind)];image_=image[static_cast<int>(kind)];unique_=true;}
std::unique_ptr<Item> ClassArmor::clone()const{return std::unique_ptr<Item>(new ClassArmor(*this));}
std::vector<ItemAction> ClassArmor::actions(const Hero&hero)const{std::vector<ItemAction>result=Armor::actions(hero);if(hero.belongings().armor()==this&&hero.HP>=3){switch(kind_){case ClassArmorKind::Warrior:result.insert(result.begin(),ItemAction::HeroicLeap);break;case ClassArmorKind::Mage:result.insert(result.begin(),ItemAction::MoltenEarth);break;case ClassArmorKind::Rogue:result.insert(result.begin(),ItemAction::SmokeBomb);break;case ClassArmorKind::Huntress:result.insert(result.begin(),ItemAction::SpectralBlades);break;}}return result;}
PreparedFood::PreparedFood(PreparedFoodKind kind):kind_(kind){static const char*n[]={"chargrilled meat","frozen carpaccio","overpriced ration"};static const int image[]={114,116,115};name_=n[static_cast<int>(kind)];image_=image[static_cast<int>(kind)];setEnergy(kind==PreparedFoodKind::OverpricedRation?200.0f:150.0f);}
std::unique_ptr<Item> PreparedFood::clone()const{return std::unique_ptr<Item>(new PreparedFood(*this));}
WandHolster::WandHolster():Bag(12,"wand holster"){image_=ItemSpriteSheet::HOLSTER;unique_=true;}
bool WandHolster::grab(const Item&item)const{return dynamic_cast<const Wand*>(&item)!=nullptr;}
std::unique_ptr<Item> WandHolster::clone()const{std::unique_ptr<WandHolster> out(new WandHolster());for(const auto&i:items_)if(i)out->collect(i->clone());return std::unique_ptr<Item>(out.release());}
}
