#include "game/levels/painters/BossExitPainter.h"
#include "game/levels/painters/Painter.h"
#include "game/levels/Level.h"
#include "game/levels/Room.h"
#include "game/levels/Terrain.h"

namespace pdv {
void BossExitPainter::paint(Level& level, Room& room) {
    Painter::fill(level, room, Terrain::WALL);
    Painter::fill(level, room, 1, Terrain::EMPTY);
    for (auto& door : room.doors()) if (door) door->set(Room::Door::Type::Regular);
    const int exit = room.top * Level::Width + (room.left + room.right) / 2;
    level.setExit(exit);
    level.setRaw(exit, Terrain::LOCKED_EXIT);
}
}
