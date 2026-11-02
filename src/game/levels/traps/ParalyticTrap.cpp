#include "game/levels/traps/ParalyticTrap.h"
#include "game/Dungeon.h"
#include "game/actors/blobs/Blob.h"
namespace pdv {
void ParalyticTrap::trigger(Dungeon& dungeon, int pos, Char*) {
    dungeon.level().seedBlob(
        BlobKind::ParalyticGas,
        pos,
        80 + 5 * dungeon.depth()
    );
}
}
