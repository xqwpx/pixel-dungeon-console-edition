#include "game/mechanics/Ballistica.h"
#include "game/Dungeon.h"
#include <algorithm>
#include <stdlib.h>
namespace pdv {
std::array<int, Level::Width> Ballistica::trace_{};
int Ballistica::distance_=0;
int Ballistica::cast(const Dungeon& dungeon,int from,int to,bool magic,bool hitChars){
    const Level& level=dungeon.level();
    const int w=Level::Width;
    int x0=from%w,x1=to%w,y0=from/w,y1=to/w;
    int dx=x1-x0,dy=y1-y0;
    int stepX=dx>0?1:-1,stepY=dy>0?1:-1;
    dx=std::abs(dx);dy=std::abs(dy);
    int stepA,stepB,dA,dB;
    if(dx>dy){stepA=stepX;stepB=stepY*w;dA=dx;dB=dy;}
    else{stepA=stepY*w;stepB=stepX;dA=dy;dB=dx;}
    distance_=1;trace_[0]=from;int cell=from;int err=dA/2;
    while(cell!=to || magic){
        cell+=stepA;err+=dB;
        if(err>=dA){err-=dA;cell+=stepB;}
        if(distance_>=static_cast<int>(trace_.size())) return trace_[distance_-1];
        trace_[distance_++]=cell;
        if(!level.inBoundsCell(cell)) return trace_[distance_-2];
        if(!level.passable(cell) && !level.avoid(cell)) return trace_[--distance_-1];
        if(level.losBlocking(cell) || (hitChars && dungeon.scheduler().findChar(cell)!=nullptr)) return cell;
    }
    if(distance_<static_cast<int>(trace_.size())) trace_[distance_++]=cell;
    return to;
}
} // namespace pdv
