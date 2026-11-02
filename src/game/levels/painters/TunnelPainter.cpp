#include "game/levels/painters/TunnelPainter.h"
#include "game/levels/painters/Painter.h"
#include "game/levels/Level.h"
#include "game/levels/Room.h"
#include "game/utils/Random.h"
namespace pdv {
void TunnelPainter::paint(Level&level,Room&room){
 int floor=level.tunnelTile();Point c=room.center();
 if(room.width()>room.height() || (room.width()==room.height()&&Random::Int(2)==0)){
  int from=room.right-1,to=room.left+1;
  for(auto&kv:room.connected){auto door=kv.second;if(!door)continue;int step=door->y<c.y?1:-1;
   if(door->x==room.left){from=room.left+1;for(int i=door->y;i!=c.y;i+=step)Painter::set(level,from,i,floor);}
   else if(door->x==room.right){to=room.right-1;for(int i=door->y;i!=c.y;i+=step)Painter::set(level,to,i,floor);}
   else {if(door->x<from)from=door->x;if(door->x>to)to=door->x;for(int i=door->y+step;i!=c.y;i+=step)Painter::set(level,door->x,i,floor);}
  }
  for(int i=from;i<=to;++i)Painter::set(level,i,c.y,floor);
 } else {
  int from=room.bottom-1,to=room.top+1;
  for(auto&kv:room.connected){auto door=kv.second;if(!door)continue;int step=door->x<c.x?1:-1;
   if(door->y==room.top){from=room.top+1;for(int i=door->x;i!=c.x;i+=step)Painter::set(level,i,from,floor);}
   else if(door->y==room.bottom){to=room.bottom-1;for(int i=door->x;i!=c.x;i+=step)Painter::set(level,i,to,floor);}
   else {if(door->y<from)from=door->y;if(door->y>to)to=door->y;for(int i=door->x+step;i!=c.x;i+=step)Painter::set(level,i,door->y,floor);}
  }
  for(int i=from;i<=to;++i)Painter::set(level,c.x,i,floor);
 }
 for(auto&kv:room.connected)if(kv.second)kv.second->set(Room::Door::Type::Tunnel);
}}
