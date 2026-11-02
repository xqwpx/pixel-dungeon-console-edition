#include "game/levels/painters/PassagePainter.h"
#include "game/levels/painters/Painter.h"
#include "game/levels/Level.h"
#include "game/levels/Room.h"
#include <algorithm>
#include <limits>
#include <vector>

namespace pdv {
namespace {
int perimeterIndex(const Room& room, const Room::Door& door) {
    const int width = room.width() - 2;
    const int height = room.height() - 2;
    if (door.y == room.top) return door.x - room.left - 1;
    if (door.x == room.right) return width + door.y - room.top - 1;
    if (door.y == room.bottom) return width + height + room.right - door.x - 1;
    return door.y == room.top + 1 ? 0 : width * 2 + height + room.bottom - door.y - 1;
}

Point perimeterPoint(const Room& room, int index) {
    const int width = room.width() - 2;
    const int height = room.height() - 2;
    if (index < width) return Point(room.left + 1 + index, room.top + 1);
    if (index < width + height) return Point(room.right - 1, room.top + 1 + index - width);
    if (index < width * 2 + height) return Point(room.right - 1 - (index - width - height), room.bottom - 1);
    return Point(room.left + 1, room.bottom - 1 - (index - width * 2 - height));
}
}

void PassagePainter::paint(Level& level, Room& room) {
    const int width = room.width() - 2;
    const int height = room.height() - 2;
    const int perimeter = width * 2 + height * 2;
    if (width <= 0 || height <= 0 || perimeter <= 0) return;

    std::vector<int> joints;
    for (const auto& door : room.doors()) if (door) joints.push_back(perimeterIndex(room, *door));
    if (joints.empty()) return;
    std::sort(joints.begin(), joints.end());

    int start = 0;
    int longestGap = joints.front() + perimeter - joints.back();
    for (std::size_t i = 1; i < joints.size(); ++i) {
        const int gap = joints[i] - joints[i - 1];
        if (gap > longestGap) { longestGap = gap; start = static_cast<int>(i); }
    }
    const int end = (start + static_cast<int>(joints.size()) - 1) % static_cast<int>(joints.size());
    for (int index = joints[start];; index = (index + 1) % perimeter) {
        Painter::set(level, perimeterPoint(room, index), level.tunnelTile());
        if (index == joints[end]) break;
    }
    for (const auto& door : room.doors()) if (door) door->set(Room::Door::Type::Tunnel);
}
}
