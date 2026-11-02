#pragma once
#include "game/items/Item.h"
#include "game/items/armor/Armor.h"
#include "game/items/food/Food.h"
#include "game/items/Bag.h"
#include "game/items/weapon/Weapon.h"
namespace pdv {
enum class SpecialKind{Amulet,Ankh,ArmorKit,Dewdrop,DewVial,LloydsBeacon,TomeOfMastery,Torch,Weightstone,CorpseDust,DarkGold,DriedRose,DwarfToken,PhantomFish,Pickaxe,RatSkull,NPCBones,ChestPlaceholder};
class SpecialItem:public Item{public:explicit SpecialItem(SpecialKind kind);SpecialKind kind()const{return kind_;}bool sameKind(const Item&o)const override{const SpecialItem*v=dynamic_cast<const SpecialItem*>(&o);return v&&v->kind_==kind_;}int state()const{return state_;}void restoreState(int value){state_=value;}std::unique_ptr<Item> clone()const override;bool isUpgradable()const override{return false;}bool isIdentified()const override{return true;}int price()const override;std::vector<ItemAction> actions(const Hero&)const override;private:SpecialKind kind_;int state_=-1;};
class QuestPickaxe:public Weapon{public:QuestPickaxe();int min()const override{return 3;}int max()const override{return 12;}bool bloodStained()const{return bloodStained_;}void stain(){bloodStained_=true;}void restoreBloodStained(bool value){bloodStained_=value;}bool isUpgradable()const override{return false;}bool isIdentified()const override{return true;}std::unique_ptr<Item>clone()const override;std::vector<ItemAction>actions(const Hero&)const override;private:bool bloodStained_=false;};
enum class ClassArmorKind{Warrior,Mage,Rogue,Huntress};
class ClassArmor:public Armor{public:explicit ClassArmor(ClassArmorKind kind);ClassArmorKind kind()const{return kind_;}std::unique_ptr<Item> clone()const override;std::vector<ItemAction> actions(const Hero&)const override;bool isUpgradable()const override{return false;}int price()const override{return 0;}private:ClassArmorKind kind_;};
enum class PreparedFoodKind{ChargrilledMeat,FrozenCarpaccio,OverpricedRation};
class PreparedFood:public Food{public:explicit PreparedFood(PreparedFoodKind kind);PreparedFoodKind kind()const{return kind_;}bool sameKind(const Item&o)const override{const PreparedFood*v=dynamic_cast<const PreparedFood*>(&o);return v&&v->kind_==kind_;}int price()const override{return (kind_==PreparedFoodKind::ChargrilledMeat?5:(kind_==PreparedFoodKind::FrozenCarpaccio?10:20))*quantity();}std::unique_ptr<Item> clone()const override;private:PreparedFoodKind kind_;};
class WandHolster:public Bag{public:WandHolster();bool grab(const Item&item)const override;int price()const override{return 50;}std::unique_ptr<Item> clone()const override;};
}
