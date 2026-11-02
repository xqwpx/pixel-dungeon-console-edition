#include "game/actors/blobs/ConfusionGas.h"
#include "game/Dungeon.h"
#include "game/actors/Char.h"
#include "game/actors/buffs/GasesImmunity.h"
#include "game/actors/buffs/Vertigo.h"
namespace pdv { void ConfusionGas::evolve(){Blob::evolve();if(!dungeon())return;for(int i=0;i<LevelLength;++i)if(cur_[static_cast<std::size_t>(i)]>0){Char*ch=dungeon()->scheduler().findChar(i);if(ch&&!ch->hasBuff(typeid(GasesImmunity)))Buff::prolong<Vertigo>(*ch,Vertigo::Duration);}} }
