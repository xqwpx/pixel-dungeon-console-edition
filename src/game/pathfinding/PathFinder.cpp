#include "game/pathfinding/PathFinder.h"
#include <algorithm>
#include <limits>

namespace pdv {
namespace {
int gWidth=0, gHeight=0, gSize=0;
std::vector<int> gDistance;
std::vector<int> gQueue;
std::vector<int> gDir;

bool canStep(int from, int to, const std::array<bool,LevelLength>& passable) {
    if (from < 0 || to < 0 || from >= gSize || to >= gSize) return false;
    const int fromX = from % gWidth, fromY = from / gWidth;
    const int toX = to % gWidth, toY = to / gWidth;
    const int dx = toX - fromX, dy = toY - fromY;
    if (std::abs(dx) > 1 || std::abs(dy) > 1 || (dx == 0 && dy == 0)) return false;

    // A diagonal cannot squeeze through the closed corner formed by two walls.
    if (dx != 0 && dy != 0) {
        const int horizontal = from + dx;
        const int vertical = from + dy * gWidth;
        if (!passable[static_cast<std::size_t>(horizontal)] ||
            !passable[static_cast<std::size_t>(vertical)]) return false;
    }
    return true;
}
}

void PathFinder::setMapSize(int width, int height) {
    const int size=width*height;
    if (gSize != size || gWidth != width) {
        gWidth=width; gHeight=height; gSize=size;
        gDistance.assign(size,0);
        gQueue.assign(size,0);
        gDir={-1,+1,-width,+width,-width-1,-width+1,+width-1,+width+1};
    }
}

const std::vector<int>& PathFinder::distance() { return gDistance; }

bool PathFinder::buildDistanceMapInternal(int from, int to, const std::array<bool,LevelLength>& passable) {
    if (from==to || from<0 || to<0 || from>=gSize || to>=gSize) return false;
    std::fill(gDistance.begin(),gDistance.end(),std::numeric_limits<int>::max());
    bool found=false; int head=0, tail=0;
    gQueue[tail++]=to; gDistance[to]=0;
    while (head<tail) {
        const int step=gQueue[head++];
        if (step==from) { found=true; break; }
        const int nd=gDistance[step]+1;
        for (int off:gDir) {
            const int n=step+off;
            if (canStep(step,n,passable) && (n==from || (passable[static_cast<std::size_t>(n)] && gDistance[n]>nd))) {
                if (tail<gSize) gQueue[tail++]=n;
                gDistance[n]=nd;
            }
        }
    }
    return found;
}

std::vector<int> PathFinder::find(int from, int to, const std::array<bool,LevelLength>& passable) {
    std::vector<int> result;
    if (!buildDistanceMapInternal(from,to,passable)) return result;
    int s=from;
    do {
        int minD=gDistance[s], best=s;
        for (int off:gDir) {
            const int n=s+off;
            if (!canStep(s,n,passable)) continue;
            const int d=gDistance[n];
            if (d<minD) { minD=d; best=n; }
        }
        if (best==s) { result.clear(); return result; }
        s=best; result.push_back(s);
    } while (s!=to);
    return result;
}

int PathFinder::getStep(int from, int to, const std::array<bool,LevelLength>& passable) {
    if (!buildDistanceMapInternal(from,to,passable)) return -1;
    int minD=gDistance[from], best=from;
    for (int off:gDir) {
        const int n=from+off;
        if (!canStep(from,n,passable)) continue;
        const int d=gDistance[n];
        if (d<minD) { minD=d; best=n; }
    }
    return best;
}

void PathFinder::buildDistanceMap(int to, const std::array<bool,LevelLength>& passable, int limit) {
    std::fill(gDistance.begin(),gDistance.end(),std::numeric_limits<int>::max());
    if (to<0 || to>=gSize) return;
    int head=0, tail=0; gQueue[tail++]=to; gDistance[to]=0;
    while (head<tail) {
        const int step=gQueue[head++];
        const int nd=gDistance[step]+1;
        if (nd>limit) return;
        for (int off:gDir) {
            const int n=step+off;
            if (canStep(step,n,passable) && passable[static_cast<std::size_t>(n)] && gDistance[n]>nd) {
                if (tail<gSize) gQueue[tail++]=n;
                gDistance[n]=nd;
            }
        }
    }
}

} // namespace pdv
