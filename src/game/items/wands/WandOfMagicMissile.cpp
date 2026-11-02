#include "game/items/wands/WandOfMagicMissile.h"
#include "game/items/ItemSpriteSheet.h"
#include "game/Dungeon.h"
#include "game/actors/Char.h"
#include "game/utils/Random.h"
namespace pdv {
WandOfMagicMissile::WandOfMagicMissile(){name_="Wand of Magic Missile";image_=ItemSpriteSheet::WAND_MAGIC_MISSILE;maxCharges_=initialCharges();curCharges_=maxCharges_;}
void WandOfMagicMissile::onZap(Dungeon& dungeon,Hero&,int cell){
    Char* ch=dungeon.scheduler().findChar(cell);
    if(ch){ int lvl=power(); ch->damage(Random::Int(1,6+lvl*2),this); }
}
std::unique_ptr<Item> WandOfMagicMissile::clone() const {std::unique_ptr<WandOfMagicMissile>copy(new WandOfMagicMissile());copy->quantity(quantity());copy->level(level());copy->setCursed(cursed());copy->setCursedKnown(cursedKnown());if(levelKnown())copy->identify();copy->restoreDurability(durability());copy->restoreCharges(curCharges());return std::move(copy);}
} // namespace pdv
