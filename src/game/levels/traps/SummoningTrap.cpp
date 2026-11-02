#include "game/levels/traps/SummoningTrap.h"
#include "game/Dungeon.h"
#include "game/actors/Char.h"
#include "game/actors/mobs/Bestiary.h"
#include "game/actors/mobs/Mob.h"
#include "game/levels/Level.h"
#include "game/utils/Random.h"
#include <vector>
namespace pdv {
void SummoningTrap::trigger(Dungeon& dungeon,int pos,Char*) {
    if (dungeon.depth()%5==0) return;
    int n=1; if(Random::Int(2)==0){++n;if(Random::Int(2)==0)++n;}
    std::vector<int> candidates;
    for(int off:Level::Neighbours8){int p=pos+off;if(dungeon.level().inBoundsCell(p)&&dungeon.scheduler().findChar(p)==nullptr&&(dungeon.level().passable(p)||dungeon.level().avoid(p)))candidates.push_back(p);}
    while(n>0&&!candidates.empty()){
        int index=Random::Int(static_cast<int>(candidates.size())); int cell=candidates[static_cast<std::size_t>(index)]; candidates.erase(candidates.begin()+index); --n;
        std::unique_ptr<Mob> mob=Bestiary::mutableMob(dungeon.depth());
        if(!mob) continue; // no fake substitution for unported depth tables
        mob->pos=cell; mob->bind(&dungeon); mob->setState(Mob::State::Wandering);
        Mob* raw=mob.get(); dungeon.level().addMob(std::move(mob)); dungeon.scheduler().addDelayed(raw,Delay);
    }
}
}
