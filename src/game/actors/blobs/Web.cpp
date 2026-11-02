#include "game/actors/blobs/Web.h"
#include "game/Dungeon.h"
#include "game/actors/Char.h"
#include "game/actors/buffs/Roots.h"
namespace pdv {void Web::seed(int cell,int amount){if(cell<0||cell>=LevelLength)return;if(amount>valueAt(cell))setValueAt(cell,amount);}void Web::evolve(){if(!dungeon())return;for(int i=0;i<LevelLength;++i){const int value=cur_[static_cast<std::size_t>(i)]>0?cur_[static_cast<std::size_t>(i)]-1:0;off_[static_cast<std::size_t>(i)]=value;volume_+=value;if(value>0)if(Char*ch=dungeon()->scheduler().findChar(i))Buff::prolong<Roots>(*ch,TICK);}}}
