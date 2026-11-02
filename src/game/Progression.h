#pragma once
#include "core/GameSession.h"
#include "game/actors/HeroSubClass.h"
#include <array>
#include <bitset>
#include <cstdint>
#include <string>
#include <vector>
#include "core/StoragePaths.h"
namespace pdv {
enum class Badge:int{
 FirstBlood,TreasureHunter,DeepExplorer,MonsterHunter,ItemCollector,BossSlayer,DungeonMaster,Victory,
 MonstersSlain1,MonstersSlain2,MonstersSlain3,MonstersSlain4,GoldCollected1,GoldCollected2,GoldCollected3,GoldCollected4,
 LevelReached1,LevelReached2,LevelReached3,LevelReached4,AllPotionsIdentified,AllScrollsIdentified,AllRingsIdentified,AllWandsIdentified,AllItemsIdentified,
 BagSeedPouch,BagScrollHolder,BagWandHolster,AllBagsBought,DeathFire,DeathPoison,DeathGas,DeathHunger,DeathGlyph,DeathFalling,YASD,
 Boss1Warrior,Boss1Mage,Boss1Rogue,Boss1Huntress,Boss1,Boss2,Boss3,Boss4,Boss1AllClasses,
 Boss3Gladiator,Boss3Berserker,Boss3Warlock,Boss3Battlemage,Boss3Freerunner,Boss3Assassin,Boss3Sniper,Boss3Warden,Boss3AllSubclasses,
 RingHaggler,RingThorns,Strength1,Strength2,Strength3,Strength4,Food1,Food2,Food3,Food4,
 MasteryWarrior,MasteryMage,MasteryRogue,MasteryHuntress,ItemLevel1,ItemLevel2,ItemLevel3,ItemLevel4,
 RareAlbino,RareBandit,RareShielded,RareSenior,RareAcidic,Rare,VictoryWarrior,VictoryMage,VictoryRogue,VictoryHuntress,VictoryAllClasses,
 MasteryCombo,PotionsCooked1,PotionsCooked2,PotionsCooked3,PotionsCooked4,NoMonstersSlain,GrimWeapon,Piranhas,NightHunter,
 GamesPlayed1,GamesPlayed2,GamesPlayed3,GamesPlayed4,HappyEnd,Champion,Supporter,Count
};
enum class JournalFeature:int{WellHealth,WellAwareness,WellTransmutation,SacrificialFire,Alchemy,Garden,Statue,Ghost,Wandmaker,Troll,Imp};
struct JournalRecord{JournalFeature feature=JournalFeature::WellHealth;int depth=1;};
struct RankingEntry{int score=0;int depth=0;int heroClass=0;int kills=0;int gold=0;float duration=0;bool won=false;};
struct RunStatistics{int depth=1,kills=0,gold=0,items=0,food=0,bosses=0,potionsCooked=0,piranhasKilled=0,nightHunt=0,ankhsUsed=0,maxHeroLevel=1,maxStrength=10;float duration=0;bool qualifiedForNoKilling=true,completedWithNoKilling=false,amuletObtained=false;HeroClass heroClass=HeroClass::Warrior;};
class Progression{public:static Progression&instance();void load(const std::string&path=StoragePaths::profile());void save(const std::string&path=StoragePaths::profile())const;void beginRun(HeroClass);void restoreRun(const RunStatistics&run,const std::vector<JournalRecord>&journal){run_=run;journal_=journal;}void noteDepth(int);void noteHeroLevel(int);void noteStrength(int);void noteTime(float);void noteKill(bool boss,HeroSubClass subClass=HeroSubClass::None);void noteGold(int);void noteItem(int level=0);void noteFood();void notePotionCooked();void notePiranha();void noteAnkh();void noteCombo(int);void noteRare(int);void noteBag(int);void noteHaggler();void noteThorns();void noteGrimWeapon();void noteMastery(HeroClass);void notePotionIdentified();void noteScrollIdentified();void noteRingIdentified();void noteWandIdentified();void noteDeath(Badge);void noteHappyEnd();void noteAmuletObtained(){run_.amuletObtained=true;}void finishRun(bool won);void addJournal(JournalFeature,int);void removeJournal(JournalFeature,int);void resetForTests();const RunStatistics&run()const{return run_;}int runs()const{return runs_;}int wins()const{return wins_;}int deepest()const{return deepest_;}bool hasBadge(Badge badge)const{return badges_.test(static_cast<std::size_t>(badge));}bool takeUnlockedBadge(Badge& badge);uint32_t badges()const{uint32_t value=0;for(int i=0;i<32;++i)if(badges_.test(static_cast<std::size_t>(i)))value|=1u<<i;return value;}const std::array<RankingEntry,10>&rankings()const{return rankings_;}const std::vector<JournalRecord>&journal()const{return journal_;}std::string badgeName(Badge)const;private:void award(Badge);void validateRunThresholds();void insertRanking(const RankingEntry&);RunStatistics run_{};int runs_=0,wins_=0,deepest_=0,totalKills_=0,totalGold_=0,totalItems_=0;std::bitset<128>badges_{};std::vector<Badge>unlockedBadges_{};std::array<RankingEntry,10>rankings_{};std::vector<JournalRecord>journal_;};
}
