#include "game/items/potions/PotionOfLiquidFlame.h"
#include "game/items/ItemSpriteSheet.h"
#include "game/Dungeon.h"
#include "game/actors/blobs/Blob.h"

namespace pdv {
PotionOfLiquidFlame::PotionOfLiquidFlame() {
    image_ = ItemSpriteSheet::POTION_GOLDEN;
    name_ = "Potion of Liquid Flame";
}
void PotionOfLiquidFlame::shatter(Dungeon& dungeon, int cell) {
    if (dungeon.visible()[static_cast<std::size_t>(cell)]) {
        setKnown();
    }
    dungeon.level().seedBlob(
        BlobKind::Fire,
        cell,
        2
    );
}
std::unique_ptr<Item> PotionOfLiquidFlame::clone() const {
    return std::unique_ptr<Item>(new PotionOfLiquidFlame(*this));
}
}
