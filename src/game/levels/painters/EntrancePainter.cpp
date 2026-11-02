#include "game/levels/painters/EntrancePainter.h"
#include "game/levels/painters/Painter.h"
#include "game/levels/Level.h"
#include "game/levels/Room.h"
#include "game/levels/Terrain.h"
namespace pdv { void EntrancePainter::paint(Level&l,Room&r){
 Painter::fill(l,r,Terrain::WALL);Painter::fill(l,r,1,Terrain::EMPTY);
 for(auto&d:r.doors())d->set(Room::Door::Type::Regular);
 Point p=r.random(1);l.setEntrance(p.y*Level::Width+p.x);Painter::set(l,p,Terrain::ENTRANCE);
}}
