#include "game/items/wands/Wand.h"
#include "game/actors/Hero.h"
#include "game/Dungeon.h"
#include "game/mechanics/Ballistica.h"
#include "game/items/rings/Ring.h"
#include "game/items/ItemSpriteSheet.h"
#include "game/items/ItemStatusHandler.h"
#include "game/Progression.h"
#include <array>
#include <algorithm>
#include <cmath>
#include <sstream>
namespace pdv {
namespace {
ItemStatusHandler gWandStatus;
const std::array<std::string, ItemStatusHandler::Capacity> kWandMaterials = {{
    "bamboo", "birch", "cedar", "ebony", "elm", "hazel",
    "oak", "pine", "purpleheart", "rowan", "teak", "yew"
}};
const std::array<int, ItemStatusHandler::Capacity> kWandImages = {{
    ItemSpriteSheet::WAND_BAMBOO, ItemSpriteSheet::WAND_PURPLEHEART, ItemSpriteSheet::WAND_OAK, ItemSpriteSheet::WAND_BIRCH,
    ItemSpriteSheet::WAND_BAMBOO, ItemSpriteSheet::WAND_PURPLEHEART, ItemSpriteSheet::WAND_OAK, ItemSpriteSheet::WAND_BIRCH,
    ItemSpriteSheet::WAND_BAMBOO, ItemSpriteSheet::WAND_PURPLEHEART, ItemSpriteSheet::WAND_OAK, ItemSpriteSheet::WAND_BIRCH
}};
}
void Wand::initAppearances(){gWandStatus.init(kWandMaterials,kWandImages);}
uint16_t Wand::knownMask(){return gWandStatus.knownMask();}
void Wand::restoreKnownMask(uint16_t mask){gWandStatus.restoreKnownMask(mask);}
bool Wand::isKnown()const{const int index=statusIndex();return index<0||gWandStatus.isKnown(index);}
void Wand::setKnown(){const int index=statusIndex();if(index>=0){gWandStatus.know(index);if(gWandStatus.allKnown())Progression::instance().noteWandIdentified();}}
Item& Wand::identify(){setKnown();return Item::identify();}
bool Wand::isIdentified()const{return isKnown()&&Item::isIdentified();}
int Wand::image()const{const int index=statusIndex();return isKnown()||index<0?Item::image():gWandStatus.image(index);}
std::string Wand::displayName()const{const int index=statusIndex();return isKnown()||index<0?Item::displayName():gWandStatus.label(index)+" wand";}
std::vector<ItemAction> Wand::actions(const Hero& hero) const {
    const bool equipped = hero.belongings().weapon() == this;
    std::vector<ItemAction> result{equipped ? ItemAction::Unequip : ItemAction::Equip,
                                   ItemAction::Drop, ItemAction::QuickSlot};
    // The C++ UI always displays the exact number of charges, unlike the
    // original's initially hidden counter.  Do not offer a dead Zap action
    // when that known counter is empty.
    if (curCharges_ > 0) result.insert(result.begin() + 1, ItemAction::Zap);
    return result;
}
std::string Wand::info() const { std::ostringstream out; out<<Item::info()<<" | charges "<<curCharges_<<"/"<<maxCharges_; return out.str(); }
Wand::Wand(){ maxCharges_=initialCharges(); curCharges_=maxCharges_; }
Wand::~Wand(){ stopCharging(); }
int Wand::power()const{int value=effectiveLevel();if(charger_&&charger_->hero())for(const Ring*ring:{charger_->hero()->belongings().ring1(),charger_->hero()->belongings().ring2()})if(ring)value+=ring->powerBonus();return std::max(0,value);}
int Wand::min() const { int tier=1+effectiveLevel()/3; return tier; }
int Wand::max() const { int lvl=effectiveLevel(); int tier=1+lvl/3; return (tier*tier-tier+10)/2+lvl; }
Item& Wand::upgrade(int n){
    for(int i=0;i<n;i++){ Item::upgrade(1); updateLevel(); curCharges_=std::min(curCharges_+1,maxCharges_); }
    return *this;
}
Item& Wand::degrade(int n){
    for(int i=0;i<n;i++){ Item::degrade(1); updateLevel(); }
    return *this;
}
void Wand::activate(Hero& hero, ActorScheduler& scheduler){
    if(charger_) return;
    charger_.reset(new Charger()); charger_->bind(this,&hero); scheduler.add(charger_.get());
}
void Wand::stopCharging(){
    if(charger_ && charger_->attachedScheduler()) charger_->attachedScheduler()->remove(charger_.get());
    charger_.reset();
}
float Wand::rechargeDelay(const Hero& hero) const {
    float time=40.0f;
    if(hero.heroClass()==HeroClass::Mage) time/=std::sqrt(1.0f+static_cast<float>(effectiveLevel()));
    return time;
}
void Wand::Charger::bind(Wand* wand, Hero* hero){
    wand_=wand; hero_=hero; resetTime();
    spend(wand_ && hero_ ? wand_->rechargeDelay(*hero_) : 40.0f);
}
bool Wand::Charger::act(){
    if(!wand_ || !hero_){ next(); return true; }
    if(wand_->curCharges_<wand_->maxCharges_) wand_->curCharges_++;
    spend(wand_->rechargeDelay(*hero_)); next(); return true;
}
bool Wand::zap(Dungeon& dungeon,Hero& hero,int target){
    if(target==hero.pos) return false;
    int cell=Ballistica::cast(dungeon,hero.pos,target,true,hitChars_);
    // A wand's appearance is identified as soon as its user tries to zap it,
    // matching the original CellSelector path.
    setKnown();
    if(curCharges_>0){ onZap(dungeon,hero,cell); curCharges_--; use(); }
    hero.spendAndNext(1.0f); return true;
}
void Wand::updateLevel(){ maxCharges_=std::min(initialCharges()+level_,9); curCharges_=std::min(curCharges_,maxCharges_); }
} // namespace pdv
#include <sstream>
