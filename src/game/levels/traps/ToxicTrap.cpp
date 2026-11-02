#include "game/levels/traps/ToxicTrap.h"
#include "game/Dungeon.h"
#include "game/actors/blobs/Blob.h"
namespace pdv {
void ToxicTrap::trigger(Dungeon& dungeon, int pos, Char*) {
    dungeon.level().seedBlob(
        BlobKind::ToxicGas,
        pos,
        300 + 20 * dungeon.depth()
    );
}
}
