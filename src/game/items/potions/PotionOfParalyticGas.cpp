#include "game/items/potions/PotionOfParalyticGas.h"
#include "game/items/ItemSpriteSheet.h"
#include "game/Dungeon.h"
#include "game/actors/blobs/Blob.h"

namespace pdv {
PotionOfParalyticGas::PotionOfParalyticGas() {
    image_ = ItemSpriteSheet::POTION_JADE;
    name_ = "Potion of Paralytic Gas";
}
void PotionOfParalyticGas::shatter(Dungeon& dungeon, int cell) {
    if (dungeon.visible()[static_cast<std::size_t>(cell)]) {
        setKnown();
    }
    dungeon.level().seedBlob(
        BlobKind::ParalyticGas,
        cell,
        1000
    );
}
std::unique_ptr<Item> PotionOfParalyticGas::clone() const {
    return std::unique_ptr<Item>(new PotionOfParalyticGas(*this));
}
}
