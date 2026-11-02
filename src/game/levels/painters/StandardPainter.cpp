#include "game/levels/painters/StandardPainter.h"
#include "game/levels/painters/Painter.h"
#include "game/levels/Level.h"
#include "game/levels/Room.h"
#include "game/levels/Terrain.h"
#include "game/utils/Random.h"
#include <algorithm>
#include <stdlib.h>
namespace pdv {
static void graveyard(Level&l,Room&r){Painter::fill(l,r.left+1,r.top+1,r.width()-1,r.height()-1,Terrain::GRASS);}
static void burned(Level&l,Room&r){
 for(int y=r.top+1;y<r.bottom;++y)for(int x=r.left+1;x<r.right;++x){
  int t=Terrain::EMBERS;switch(Random::Int(5)){case 0:t=Terrain::EMPTY;break;case 1:t=Terrain::FIRE_TRAP;break;
  case 2:t=Terrain::SECRET_FIRE_TRAP;break;case 3:t=Terrain::INACTIVE_TRAP;break;}Painter::set(l,x,y,t);
 }
}
static void striped(Level&l,Room&r){
 Painter::fill(l,r.left+1,r.top+1,r.width()-1,r.height()-1,Terrain::EMPTY_SP);
 if(r.width()>r.height())for(int x=r.left+2;x<r.right;x+=2)Painter::fill(l,x,r.top+1,1,r.height()-1,Terrain::HIGH_GRASS);
 else for(int y=r.top+2;y<r.bottom;y+=2)Painter::fill(l,r.left+1,y,r.width()-1,1,Terrain::HIGH_GRASS);
}
static void study(Level&l,Room&r){
 Painter::fill(l,r.left+1,r.top+1,r.width()-1,r.height()-1,Terrain::BOOKSHELF);
 Painter::fill(l,r.left+2,r.top+2,r.width()-3,r.height()-3,Terrain::EMPTY_SP);
 for(auto&kv:r.connected){auto d=kv.second;if(!d)continue;
  if(d->x==r.left)Painter::set(l,d->x+1,d->y,Terrain::EMPTY);else if(d->x==r.right)Painter::set(l,d->x-1,d->y,Terrain::EMPTY);
  else if(d->y==r.top)Painter::set(l,d->x,d->y+1,Terrain::EMPTY);else if(d->y==r.bottom)Painter::set(l,d->x,d->y-1,Terrain::EMPTY);
 }
 Painter::set(l,r.center(),Terrain::PEDESTAL);
}
static void bridge(Level&l,Room&r,int depth){
 Painter::fill(l,r.left+1,r.top+1,r.width()-1,r.height()-1,((depth%5)!=0&&((depth+1)%5)!=0&&Random::Int(3)==0)?Terrain::CHASM:Terrain::WATER);
 std::vector<std::shared_ptr<Room::Door>> ds=r.doors();if(ds.size()!=2||!ds[0]||!ds[1])return;
 auto d1=ds[0],d2=ds[1];
 if((d1->x==r.left&&d2->x==r.right)||(d1->x==r.right&&d2->x==r.left)){
  int n=r.width()/2;Painter::drawInside(l,r,Point(d1->x,d1->y),n,Terrain::EMPTY_SP);Painter::drawInside(l,r,Point(d2->x,d2->y),n,Terrain::EMPTY_SP);
  Painter::fill(l,r.center().x,std::min(d1->y,d2->y),1,std::abs(d1->y-d2->y)+1,Terrain::EMPTY_SP);
 } else if((d1->y==r.top&&d2->y==r.bottom)||(d1->y==r.bottom&&d2->y==r.top)){
  int n=r.height()/2;Painter::drawInside(l,r,Point(d1->x,d1->y),n,Terrain::EMPTY_SP);Painter::drawInside(l,r,Point(d2->x,d2->y),n,Terrain::EMPTY_SP);
  Painter::fill(l,std::min(d1->x,d2->x),r.center().y,std::abs(d1->x-d2->x)+1,1,Terrain::EMPTY_SP);
 } else if(d1->x==d2->x)Painter::fill(l,d1->x==r.left?r.left+1:r.right-1,std::min(d1->y,d2->y),1,std::abs(d1->y-d2->y)+1,Terrain::EMPTY_SP);
 else if(d1->y==d2->y)Painter::fill(l,std::min(d1->x,d2->x),d1->y==r.top?r.top+1:r.bottom-1,std::abs(d1->x-d2->x)+1,1,Terrain::EMPTY_SP);
 else if(d1->y==r.top||d1->y==r.bottom){Painter::drawInside(l,r,Point(d1->x,d1->y),std::abs(d1->y-d2->y),Terrain::EMPTY_SP);Painter::drawInside(l,r,Point(d2->x,d2->y),std::abs(d1->x-d2->x),Terrain::EMPTY_SP);}
 else if(d1->x==r.left||d1->x==r.right){Painter::drawInside(l,r,Point(d1->x,d1->y),std::abs(d1->x-d2->x),Terrain::EMPTY_SP);Painter::drawInside(l,r,Point(d2->x,d2->y),std::abs(d1->y-d2->y),Terrain::EMPTY_SP);}
}
static void fissure(Level&l,Room&r){
 Painter::fill(l,r.left+1,r.top+1,r.width()-1,r.height()-1,Terrain::EMPTY);
 for(int y=r.top+2;y<r.bottom-1;++y)for(int x=r.left+2;x<r.right-1;++x){
  int v=std::min(y-r.top,r.bottom-y),h=std::min(x-r.left,r.right-x);
  if(std::min(v,h)>2||Random::Int(2)==0)Painter::set(l,x,y,Terrain::CHASM);
 }
}
void StandardPainter::paint(Level&l,Room&r,int depth){
 Painter::fill(l,r,Terrain::WALL);for(auto&kv:r.connected)if(kv.second)kv.second->set(Room::Door::Type::Regular);
 if(depth%5!=0&&Random::Int(5)==0){
  switch(Random::Int(6)){
   case 0:if(l.feeling()!=Level::Feeling::Grass&&std::min(r.width(),r.height())>=4&&std::max(r.width(),r.height())>=6){graveyard(l,r);return;}
   /* fall through */
   case 1:if(depth>1){burned(l,r);return;}break;
   case 2:if(std::max(r.width(),r.height())>=4){striped(l,r);return;}break;
   case 3:if(r.width()>=6&&r.height()>=6){study(l,r);return;}break;
   case 4:if(l.feeling()!=Level::Feeling::Water&&r.connected.size()==2&&r.width()>=4&&r.height()>=4){bridge(l,r,depth);return;}
   /* fall through */
   case 5:if(depth%5!=0&&(depth+1)%5!=0&&std::min(r.width(),r.height())>=5){fissure(l,r);return;}break;
  }
 }
 Painter::fill(l,r,1,Terrain::EMPTY);
}}
