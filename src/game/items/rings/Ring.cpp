#include "game/items/rings/Ring.h"
#include "game/actors/Hero.h"
#include "game/items/ItemSpriteSheet.h"
#include "game/items/ItemStatusHandler.h"
#include "game/Progression.h"
#include <array>
namespace pdv {
namespace {
ItemStatusHandler gRingStatus;
const std::array<std::string, ItemStatusHandler::Capacity> kGemNames = {{
    "agate", "amethyst", "diamond", "emerald", "garnet", "jade",
    "opal", "quartz", "ruby", "sapphire", "topaz", "onyx"
}};
const std::array<int, ItemStatusHandler::Capacity> kGemImages = {{
    ItemSpriteSheet::RING, ItemSpriteSheet::RING_EMERALD, ItemSpriteSheet::RING_SAPPHIRE, ItemSpriteSheet::RING_QUARTZ,
    ItemSpriteSheet::RING_AGATE, ItemSpriteSheet::RING, ItemSpriteSheet::RING_EMERALD, ItemSpriteSheet::RING_SAPPHIRE,
    ItemSpriteSheet::RING_QUARTZ, ItemSpriteSheet::RING_AGATE, ItemSpriteSheet::RING, ItemSpriteSheet::RING_EMERALD
}};
}
void Ring::initGems(){gRingStatus.init(kGemNames,kGemImages);}
uint16_t Ring::knownMask(){return gRingStatus.knownMask();}
void Ring::restoreKnownMask(uint16_t mask){gRingStatus.restoreKnownMask(mask);}
bool Ring::isKnown()const{const int index=statusIndex();return index<0||gRingStatus.isKnown(index);}
void Ring::setKnown(){const int index=statusIndex();if(index>=0){gRingStatus.know(index);if(gRingStatus.allKnown())Progression::instance().noteRingIdentified();}}
Item& Ring::identify(){setKnown();return Item::identify();}
bool Ring::isIdentified()const{return isKnown()&&Item::isIdentified();}
int Ring::image()const{const int index=statusIndex();return isKnown()||index<0?Item::image():gRingStatus.image(index);}
std::string Ring::displayName()const{const int index=statusIndex();return isKnown()||index<0?Item::displayName():gRingStatus.label(index)+" ring";}
float elementsDurationFactor(const Char&ch){const Hero*hero=dynamic_cast<const Hero*>(&ch);if(!hero)return 1.0f;int level=0;for(const Ring*ring:{hero->belongings().ring1(),hero->belongings().ring2()})if(ring)level+=ring->elementsBonus();return level<0?1.0f:(2.0f+0.5f*level)/(2.0f+level);}
std::vector<ItemAction> Ring::actions(const Hero& hero) const {
    const bool equipped=hero.belongings().ring1()==this || hero.belongings().ring2()==this;
    return {equipped?ItemAction::Unequip:ItemAction::Equip,ItemAction::Drop};
}
}
