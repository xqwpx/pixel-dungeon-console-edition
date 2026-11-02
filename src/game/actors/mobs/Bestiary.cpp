#include "game/actors/mobs/Bestiary.h"

#include "game/actors/mobs/Rat.h"
#include "game/actors/mobs/Gnoll.h"
#include "game/actors/mobs/Crab.h"
#include "game/actors/mobs/Goo.h"
#include "game/actors/mobs/Swarm.h"
#include "game/actors/mobs/Albino.h"
#include "game/actors/mobs/Skeleton.h"
#include "game/actors/mobs/Thief.h"
#include "game/actors/mobs/ZoneMob.h"
#include "game/utils/Random.h"

namespace pdv {

std::unique_ptr<Mob> Bestiary::mob(int depth) {
    // Original Bestiary.mobClass(depth):
    // depth 1 -> 100% Rat.
    if (depth <= 1) return std::unique_ptr<Mob>(new Rat());
    if (depth == 2) {
        return Random::Int(2) == 0
            ? std::unique_ptr<Mob>(new Rat())
            : std::unique_ptr<Mob>(new Gnoll());
    }
    if (depth == 3) {
        const float roll=Random::Float()*4.02f;
        if(roll<1.0f)return std::unique_ptr<Mob>(new Rat());
        if(roll<3.0f)return std::unique_ptr<Mob>(new Gnoll());
        if(roll<4.0f)return std::unique_ptr<Mob>(new Crab());
        return std::unique_ptr<Mob>(new Swarm());
    }
    if (depth == 5) return std::unique_ptr<Mob>(new Goo());

    if(depth==4){
        const float roll=Random::Float()*6.04f;
        if(roll<1.0f)return std::unique_ptr<Mob>(new Rat());
        if(roll<3.0f)return std::unique_ptr<Mob>(new Gnoll());
        if(roll<6.0f)return std::unique_ptr<Mob>(new Crab());
        if(roll<6.02f)return std::unique_ptr<Mob>(new Swarm());
        if(roll<6.03f)return std::unique_ptr<Mob>(new Skeleton());
        return std::unique_ptr<Mob>(new Thief());
    }
    if(depth==10)return std::unique_ptr<Mob>(new ZoneBoss(ZoneBossKind::Tengu));
    if(depth==15)return std::unique_ptr<Mob>(new ZoneBoss(ZoneBossKind::DM300));
    if(depth==20)return std::unique_ptr<Mob>(new ZoneBoss(ZoneBossKind::King));
    if(depth==25)return std::unique_ptr<Mob>(new ZoneBoss(ZoneBossKind::Yog));
    auto choose=[](const float*w,int count){float total=0;for(int i=0;i<count;++i)total+=w[i];float roll=Random::Float(total);for(int i=0;i<count;++i){roll-=w[i];if(roll<0)return i;}return count-1;};
    if(depth<=9){static const float w6[]={4,2,1,.2f};static const float w7[]={3,1,1,1};static const float w8[]={3,2,1,1,1,.02f};static const float w9[]={3,3,1,1,.02f,.01f};const float*w=depth==6?w6:depth==7?w7:depth==8?w8:w9;const int n=depth==6?4:depth==7?4:depth==8?6:6;const int i=choose(w,n);if(i==0)return std::unique_ptr<Mob>(new Skeleton());if((depth==6&&i==1)||(depth>=7&&i==2))return std::unique_ptr<Mob>(new Thief());if((depth<=7&&i==2)||(depth>=8&&i==3))return std::unique_ptr<Mob>(new Swarm());if((depth==6&&i==3)||(depth==7&&i==1)||(depth>=8&&i==1))return std::unique_ptr<Mob>(new ZoneMob(ZoneMobKind::Shaman));if(depth==8&&i==4)return std::unique_ptr<Mob>(new ZoneMob(ZoneMobKind::Bat));if(depth==8)return std::unique_ptr<Mob>(new Gnoll());if(depth==9&&i==4)return std::unique_ptr<Mob>(new ZoneMob(ZoneMobKind::Bat));return std::unique_ptr<Mob>(new ZoneMob(ZoneMobKind::Brute));}
    if(depth<=14){static const float w11[]={1,.2f};static const float w12[]={1,1,.2f};static const float w13[]={1,3,1,1,.02f};static const float w14[]={1,3,1,4,.02f,.01f};const float*w=depth==11?w11:depth==12?w12:depth==13?w13:w14;const int n=depth==11?2:depth==12?3:depth==13?5:6;const int i=choose(w,n);if(i==0)return std::unique_ptr<Mob>(new ZoneMob(ZoneMobKind::Bat));if(i==1)return std::unique_ptr<Mob>(new ZoneMob(ZoneMobKind::Brute));if(depth==12)return std::unique_ptr<Mob>(new ZoneMob(ZoneMobKind::Spinner));if(i==2)return std::unique_ptr<Mob>(new ZoneMob(depth>=13?ZoneMobKind::Shaman:ZoneMobKind::Spinner));if(i==3)return std::unique_ptr<Mob>(new ZoneMob(ZoneMobKind::Spinner));if(i==4)return std::unique_ptr<Mob>(new ZoneMob(ZoneMobKind::Elemental));return std::unique_ptr<Mob>(new ZoneMob(ZoneMobKind::Monk));}
    if(depth<=19){static const float w16[]={1,1,.2f};static const float w17[]={1,1,1};static const float w18[]={1,2,1,1};static const float w19[]={1,2,3,1,.02f};const float*w=depth==16?w16:depth==17?w17:depth==18?w18:w19;const int n=depth<=17?3:depth==18?4:5;const int i=choose(w,n);if(i==0)return std::unique_ptr<Mob>(new ZoneMob(ZoneMobKind::Elemental));if((depth==16&&i==1)||(depth>=17&&i==2)||(depth>=18&&i==3))return std::unique_ptr<Mob>(new ZoneMob(ZoneMobKind::Warlock));if((depth<=16&&i==2)||(depth>=17&&i==1))return std::unique_ptr<Mob>(new ZoneMob(ZoneMobKind::Monk));if(depth>=18&&i==2)return std::unique_ptr<Mob>(new ZoneMob(ZoneMobKind::Golem));return std::unique_ptr<Mob>(new ZoneMob(ZoneMobKind::Succubus));}
    if(depth==21)return std::unique_ptr<Mob>(new ZoneMob(Random::Int(2)?ZoneMobKind::Succubus:ZoneMobKind::Eye));
    if(depth<=24){static const float w22[]={1,1};static const float w23[]={1,2,1};static const float w24[]={1,2,3};const float*w=depth==22?w22:depth==23?w23:w24;const int i=choose(w,depth==22?2:3);if(i==0)return std::unique_ptr<Mob>(new ZoneMob(ZoneMobKind::Succubus));if(i==1)return std::unique_ptr<Mob>(new ZoneMob(ZoneMobKind::Eye));return std::unique_ptr<Mob>(new ZoneMob(ZoneMobKind::Scorpio));}
    return std::unique_ptr<Mob>(new ZoneMob(ZoneMobKind::Eye));
}

std::unique_ptr<Mob> Bestiary::mutableMob(int depth) {
    std::unique_ptr<Mob> result = mob(depth);
    if (result && dynamic_cast<Rat*>(result.get()) && Random::Int(30) == 0) {
        return std::unique_ptr<Mob>(new Albino());
    }
    if(result&&Random::Int(30)==0){if(ZoneMob*z=dynamic_cast<ZoneMob*>(result.get())){if(z->kind()==ZoneMobKind::Brute)return std::unique_ptr<Mob>(new ZoneMob(ZoneMobKind::Shielded));if(z->kind()==ZoneMobKind::Monk)return std::unique_ptr<Mob>(new ZoneMob(ZoneMobKind::Senior));if(z->kind()==ZoneMobKind::Scorpio)return std::unique_ptr<Mob>(new ZoneMob(ZoneMobKind::Acidic));}if(dynamic_cast<Thief*>(result.get()))return std::unique_ptr<Mob>(new Thief(true));}
    return result;
}

} // namespace pdv
