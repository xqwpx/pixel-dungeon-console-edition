#include "game/levels/SewerLevel.h"
#include "game/levels/Terrain.h"
#include "game/Dungeon.h"
#include "game/utils/Random.h"
namespace pdv {
int SewerLevel::water() const{const int zone=(depth()-1)/5;if(zone==1||zone==3)return feeling()==Feeling::Water?65:45;if(zone==2)return feeling()==Feeling::Water?60:45;if(zone>=4)return feeling()==Feeling::Water?55:40;return feeling()==Feeling::Water?60:45;}
int SewerLevel::grass() const{const int zone=(depth()-1)/5;if(zone==2)return feeling()==Feeling::Grass?55:35;if(zone>=4)return feeling()==Feeling::Grass?55:30;return feeling()==Feeling::Grass?60:40;}
void SewerLevel::decorate(){
 if(depth()==21){
  // LastShopLevel has its own sparse City-style decoration, rather than the
  // water-edge decoration used by ordinary sewer floors.
  for(int i=0;i<Length;++i){
   if(cell(i)==Terrain::EMPTY&&Random::Int(10)==0)setRaw(i,Terrain::EMPTY_DECO);
   else if(cell(i)==Terrain::WALL&&Random::Int(8)==0)setRaw(i,Terrain::WALL_DECO);
   else if(cell(i)==Terrain::SECRET_DOOR)setRaw(i,Terrain::DOOR);
  }
  if(dungeonContext()&&dungeonContext()->questState(3)==2){
   for(const auto& room:rooms()){
    if(!room||room->type!=Room::Type::Entrance)continue;
    for(int attempt=0;attempt<20;++attempt){
     const Point point=room->random();
     const int cell=point.y*Width+point.x;
     if(cell!=entrance()){setRaw(cell,Terrain::SIGN);break;}
    }
    break;
   }
  }
  return;
 }
 for(int i=0;i<Width;++i){
  if(cell(i)==Terrain::WALL && cell(i+Width)==Terrain::WATER && Random::Int(4)==0) setRaw(i,Terrain::WALL_DECO);
 }
 for(int i=Width;i<Length-Width;++i){
  if(cell(i)==Terrain::WALL && cell(i-Width)==Terrain::WALL &&
     cell(i+Width)==Terrain::WATER && Random::Int(2)==0) setRaw(i,Terrain::WALL_DECO);
 }
 for(int i=Width+1;i<Length-Width-1;++i){
  if(cell(i)==Terrain::EMPTY){
   int count=(cell(i+1)==Terrain::WALL?1:0)+(cell(i-1)==Terrain::WALL?1:0)+
             (cell(i+Width)==Terrain::WALL?1:0)+(cell(i-Width)==Terrain::WALL?1:0);
   if(Random::Int(16)<count*count)setRaw(i,Terrain::EMPTY_DECO);
  }
 }
 for(const auto&u:rooms()){
  if(u->type!=Room::Type::Entrance)continue;
  Point p;
  do{p=u->random();}while(p.y*Width+p.x==entrance());
  setRaw(p.y*Width+p.x,Terrain::SIGN);
  break;
 }
}}
