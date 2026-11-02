#include "game/actors/blobs/Regrowth.h"
#include "game/Dungeon.h"
#include "game/actors/Char.h"
#include "game/actors/buffs/Roots.h"
#include "game/levels/Level.h"
#include "game/levels/Terrain.h"
namespace pdv { void Regrowth::evolve(){Blob::evolve();if(!dungeon())return;Level&level=dungeon()->level();bool changed=false;for(int i=0;i<LevelLength;++i)if(off_[static_cast<std::size_t>(i)]>0){const int terrain=level.cell(i);int grown=terrain;if(terrain==Terrain::EMPTY||terrain==Terrain::EMBERS||terrain==Terrain::EMPTY_DECO)grown=cur_[static_cast<std::size_t>(i)]>9?Terrain::HIGH_GRASS:Terrain::GRASS;else if(terrain==Terrain::GRASS&&cur_[static_cast<std::size_t>(i)]>9)grown=Terrain::HIGH_GRASS;if(grown!=terrain){level.setRaw(i,grown);changed=true;}if(Char*ch=dungeon()->scheduler().findChar(i))Buff::prolong<Roots>(*ch,TICK);}if(changed){level.buildFlagMaps();dungeon()->observe();}} }
