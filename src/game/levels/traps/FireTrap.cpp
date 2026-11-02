#include "game/levels/traps/FireTrap.h"
#include "game/Dungeon.h"
#include "game/actors/blobs/Blob.h"
namespace pdv {
void FireTrap::trigger(Dungeon& dungeon, int pos, Char*) {
    dungeon.level().seedBlob(
        BlobKind::Fire,
        pos,
        2
    );
}
}
