#include "game/actors/blobs/SacrificialFire.h"
#include "game/Dungeon.h"
#include "game/Progression.h"
#include "game/actors/Char.h"
#include "game/actors/Hero.h"
#include "game/actors/mobs/Mob.h"
#include "game/actors/buffs/Buff.h"
#include "game/levels/Level.h"
#include "game/items/scrolls/Scroll.h"
#include "game/utils/Random.h"
namespace pdv {
void SacrificialFire::seed(int cell,int amount){if(pos_>=0)clear(pos_);pos_=cell;Blob::seed(cell,amount);}
void SacrificialFire::evolve(){if(pos_<0)return;off_[static_cast<std::size_t>(pos_)]=cur_[static_cast<std::size_t>(pos_)];volume_=off_[static_cast<std::size_t>(pos_)];if(!dungeon())return;if(dungeon()->visible()[static_cast<std::size_t>(pos_)])Progression::instance().addJournal(JournalFeature::SacrificialFire,dungeon()->depth());if(Char* ch=dungeon()->scheduler().findChar(pos_))Buff::prolong<Marked>(*ch,Marked::Duration);}
void SacrificialFire::sacrifice(Char& ch){if(!ch.hasBuff(typeid(Marked)))return;Level* level=ch.levelContext();if(!level)return;SacrificialFire* fire=dynamic_cast<SacrificialFire*>(level->blob(BlobKind::SacrificialFire));if(!fire||fire->pos_<0)return;int exp=0;if(Mob* mob=dynamic_cast<Mob*>(&ch))exp=mob->exp()*Random::IntRange(1,3);else if(Hero* hero=dynamic_cast<Hero*>(&ch))exp=hero->maxExp();if(exp<=0)return;const int left=fire->volume_-exp;if(left>0)fire->seed(fire->pos_,left);else{fire->seed(fire->pos_,0);Progression::instance().removeJournal(JournalFeature::SacrificialFire,fire->dungeon()->depth());fire->dungeon()->level().drop(std::unique_ptr<Item>(new Scroll(ScrollKind::WipeOut)),fire->pos_);}}
}
