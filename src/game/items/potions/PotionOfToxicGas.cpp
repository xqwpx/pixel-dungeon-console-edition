#include "game/items/potions/PotionOfToxicGas.h"
#include "game/items/ItemSpriteSheet.h"
#include "game/Dungeon.h"
#include "game/actors/blobs/Blob.h"

namespace pdv {
PotionOfToxicGas::PotionOfToxicGas() {
    image_ = ItemSpriteSheet::POTION_AZURE;
    name_ = "Potion of Toxic Gas";
}
void PotionOfToxicGas::shatter(Dungeon& dungeon, int cell) {
    if (dungeon.visible()[static_cast<std::size_t>(cell)]) {
        setKnown();
    }
    dungeon.level().seedBlob(
        BlobKind::ToxicGas,
        cell,
        1000
    );
}
std::unique_ptr<Item> PotionOfToxicGas::clone() const {
    return std::unique_ptr<Item>(new PotionOfToxicGas(*this));
}
}
