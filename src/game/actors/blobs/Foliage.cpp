#include "game/actors/blobs/Foliage.h"
#include "game/Dungeon.h"
#include "game/actors/Hero.h"
#include "game/actors/buffs/Shadows.h"
#include "game/levels/Level.h"
#include "game/levels/Terrain.h"
#include "game/Progression.h"
namespace pdv { void Foliage::evolve(){if(!dungeon())return;Level&level=dungeon()->level();bool changed=false,visible=false;for(int i=Level::Width+1;i<LevelLength-Level::Width-1;++i){const int value=cur_[static_cast<std::size_t>(i)];off_[static_cast<std::size_t>(i)]=value;volume_+=value;if(value>0){visible=visible||dungeon()->visible()[static_cast<std::size_t>(i)];if(level.cell(i)==Terrain::EMBERS){level.setRaw(i,Terrain::GRASS);changed=true;}}}Hero&hero=dungeon()->hero();if(hero.isAlive()&&dungeon()->firstVisibleMobCell()<0&&valueAt(hero.pos)>0)if(Shadows* shadows=Buff::affect<Shadows>(hero))shadows->prolong();if(changed)level.buildFlagMaps();if(visible)Progression::instance().addJournal(JournalFeature::Garden,dungeon()->depth());} }
