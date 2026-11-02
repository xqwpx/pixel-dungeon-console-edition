#include "game/Progression.h"
#include <algorithm>
#include <cstddef>
#include <stdio.h>
#include <cstring>
#if defined(PD_PLATFORM_PS1)
#include "platform/ps1/PS1MemoryCard.h"
#endif
namespace pdv {
namespace {
constexpr uint32_t Magic = 0x46525044u, Version = 3;
struct StoredRankingV2 {
  int score = 0, depth = 0, heroClass = 0, kills = 0, gold = 0;
  bool won = false;
};
uint32_t sum(const unsigned char *p, std::size_t n) {
  uint32_t h = 2166136261u;
  for (std::size_t i = 0; i < n; ++i) {
    h ^= p[i];
    h *= 16777619u;
  }
  return h;
}
template <std::size_t N>
void threshold(Progression &p, int value, const int (&values)[N],
               const Badge (&badges)[N]) {
  for (std::size_t i = 0; i < N; ++i)
    if (value >= values[i]) {
      (void)p;
      (void)badges;
    }
}
void copyRankings(std::array<RankingEntry, 10> &target,
                  const StoredRankingV2 (&source)[10]) {
  for (int i = 0; i < 10; ++i) {
    target[i].score = source[i].score;
    target[i].depth = source[i].depth;
    target[i].heroClass = source[i].heroClass;
    target[i].kills = source[i].kills;
    target[i].gold = source[i].gold;
    target[i].won = source[i].won;
  }
}
} // namespace
Progression &Progression::instance() {
  static Progression value;
  return value;
}
void Progression::load(const std::string &path) {
#if defined(PD_PLATFORM_PS1)
  std::vector<unsigned char> bytes;
  if (!PS1MemoryCard::readFile(path, bytes)) return;
#else
  FILE *f = fopen(path.c_str(), "rb");
  if (!f)
    return;
  fseek(f, 0, SEEK_END);
  const long size = ftell(f);
  rewind(f);
  if (size <= 0 || size > 65536) {
    fclose(f);
    return;
  }
  std::vector<unsigned char> bytes(static_cast<std::size_t>(size));
  if (fread(bytes.data(), 1, bytes.size(), f) != bytes.size()) {
    fclose(f);
    return;
  }
  fclose(f);
#endif
  struct Header {
    uint32_t magic, version;
  };
  if (bytes.size() < sizeof(Header) + 4)
    return;
  Header h{};
  std::memcpy(&h, bytes.data(), sizeof(h));
  uint32_t stored = 0;
  std::memcpy(&stored, bytes.data() + bytes.size() - 4, 4);
  if (h.magic != Magic)
    return;
  // V1/V2 stored the checksum as the final field. V3 wrote a native struct,
  // whose trailing alignment padding follows that field.
  if (h.version != Version && stored != sum(bytes.data(), bytes.size() - 4))
    return;
  if (h.version == 1) {
    struct Old {
      uint32_t magic, version;
      int runs, wins, deepest, totalKills, totalGold, totalItems;
      uint32_t badges;
      StoredRankingV2 rankings[10];
      uint32_t checksum;
    } old{};
    if (bytes.size() != sizeof(old))
      return;
    std::memcpy(&old, bytes.data(), sizeof(old));
    badges_.reset();
    rankings_.fill(RankingEntry());
    runs_ = old.runs;
    wins_ = old.wins;
    deepest_ = old.deepest;
    totalKills_ = old.totalKills;
    totalGold_ = old.totalGold;
    totalItems_ = old.totalItems;
    for (int i = 0; i < 8; ++i)
      badges_.set(static_cast<std::size_t>(i), (old.badges & (1u << i)) != 0);
    copyRankings(rankings_, old.rankings);
    return;
  }
  if (h.version == 2) {
    struct DataV2 {
      uint32_t magic, version;
      int runs, wins, deepest, totalKills, totalGold, totalItems;
      uint64_t badges[2];
      StoredRankingV2 rankings[10];
      uint32_t checksum;
    } data{};
    if (bytes.size() != sizeof(data))
      return;
    std::memcpy(&data, bytes.data(), sizeof(data));
    rankings_.fill(RankingEntry());
    runs_ = data.runs;
    wins_ = data.wins;
    deepest_ = data.deepest;
    totalKills_ = data.totalKills;
    totalGold_ = data.totalGold;
    totalItems_ = data.totalItems;
    for (int i = 0; i < 64; ++i) {
      badges_.set(static_cast<std::size_t>(i),
                  (data.badges[0] & (uint64_t(1) << i)) != 0);
      badges_.set(static_cast<std::size_t>(i + 64),
                  (data.badges[1] & (uint64_t(1) << i)) != 0);
    }
    copyRankings(rankings_, data.rankings);
    return;
  }
  if (h.version != Version)
    return;
  struct Data {
    uint32_t magic, version;
    int runs, wins, deepest, totalKills, totalGold, totalItems;
    uint64_t badges[2];
    RankingEntry rankings[10];
    uint32_t checksum;
  } data{};
  if (bytes.size() != sizeof(data))
    return;
  std::memcpy(&data, bytes.data(), sizeof(data));
  // V3 checksum is not verifiable: its writer included this field's zero
  // placeholder in the digest, then overwrote it. Preserve load compatibility
  // until the next on-disk format version replaces this native struct.
  runs_ = data.runs;
  wins_ = data.wins;
  deepest_ = data.deepest;
  totalKills_ = data.totalKills;
  totalGold_ = data.totalGold;
  totalItems_ = data.totalItems;
  for (int i = 0; i < 64; ++i) {
    badges_.set(static_cast<std::size_t>(i),
                (data.badges[0] & (uint64_t(1) << i)) != 0);
    badges_.set(static_cast<std::size_t>(i + 64),
                (data.badges[1] & (uint64_t(1) << i)) != 0);
  }
  std::copy(std::begin(data.rankings), std::end(data.rankings),
            rankings_.begin());
}
void Progression::save(const std::string &path) const {
  struct Data {
    uint32_t magic, version;
    int runs, wins, deepest, totalKills, totalGold, totalItems;
    uint64_t badges[2];
    RankingEntry rankings[10];
    uint32_t checksum;
  } data{};
  data.magic = Magic;
  data.version = Version;
  data.runs = runs_;
  data.wins = wins_;
  data.deepest = deepest_;
  data.totalKills = totalKills_;
  data.totalGold = totalGold_;
  data.totalItems = totalItems_;
  for (int i = 0; i < 64; ++i) {
    if (badges_.test(static_cast<std::size_t>(i)))
      data.badges[0] |= uint64_t(1) << i;
    if (badges_.test(static_cast<std::size_t>(i + 64)))
      data.badges[1] |= uint64_t(1) << i;
  }
  std::copy(rankings_.begin(), rankings_.end(), std::begin(data.rankings));
  data.checksum =
      sum(reinterpret_cast<const unsigned char *>(&data), sizeof(data) - 4);
 #if defined(PD_PLATFORM_PS1)
  (void)PS1MemoryCard::writeFile(path, &data, sizeof(data));
  return;
#else
  const std::string temp = path + ".tmp";
  FILE *f = fopen(temp.c_str(), "wb");
  if (!f)
    return;
  const bool ok = fwrite(&data, 1, sizeof(data), f) == sizeof(data) &&
                  fflush(f) == 0;
  fclose(f);
  if (!ok) {
    remove(temp.c_str());
    return;
  }
  remove(path.c_str());
  rename(temp.c_str(), path.c_str());
#endif
}
void Progression::beginRun(HeroClass hero) {
  run_ = RunStatistics();
  run_.heroClass = hero;
  journal_.clear();
}
void Progression::noteTime(float time) {
  if (time > 0.0f)
    run_.duration += time;
}
void Progression::noteMastery(HeroClass hero) {
  award(static_cast<Badge>(static_cast<int>(Badge::MasteryWarrior) +
                           static_cast<int>(hero)));
}
void Progression::noteHaggler() { award(Badge::RingHaggler); }
void Progression::noteThorns() { award(Badge::RingThorns); }
void Progression::noteGrimWeapon() { award(Badge::GrimWeapon); }
void Progression::resetForTests() {
  run_ = RunStatistics();
  runs_ = wins_ = deepest_ = totalKills_ = totalGold_ = totalItems_ = 0;
  badges_.reset();
  unlockedBadges_.clear();
  rankings_.fill(RankingEntry());
  journal_.clear();
}
void Progression::award(Badge b) {
  const std::size_t index = static_cast<std::size_t>(b);
  if (badges_.test(index)) return;
  badges_.set(index);
  unlockedBadges_.push_back(b);
}

bool Progression::takeUnlockedBadge(Badge& badge) {
  if (unlockedBadges_.empty()) return false;
  badge = unlockedBadges_.front();
  unlockedBadges_.erase(unlockedBadges_.begin());
  return true;
}
void Progression::noteDepth(int depth) {
  if (depth > run_.depth) {
    run_.completedWithNoKilling = run_.qualifiedForNoKilling;
    run_.depth = depth;
  }
  const bool bossFloor =
      depth == 5 || depth == 10 || depth == 15 || depth == 20 || depth == 25;
  run_.qualifiedForNoKilling = !bossFloor;
  deepest_ = std::max(deepest_, depth);
  if (depth >= 10)
    award(Badge::DeepExplorer);
  if (depth >= 25)
    award(Badge::DungeonMaster);
}
void Progression::noteHeroLevel(int v) {
  run_.maxHeroLevel = std::max(run_.maxHeroLevel, v);
  validateRunThresholds();
}
void Progression::noteStrength(int v) {
  run_.maxStrength = std::max(run_.maxStrength, v);
  validateRunThresholds();
}
void Progression::noteKill(bool boss, HeroSubClass subClass) {
  ++run_.kills;
  ++totalKills_;
  run_.qualifiedForNoKilling = false;
  award(Badge::FirstBlood);
  if (totalKills_ >= 50)
    award(Badge::MonsterHunter);
  if (boss) {
    ++run_.bosses;
    award(Badge::BossSlayer);
    const Badge b[] = {Badge::Boss1, Badge::Boss2, Badge::Boss3, Badge::Boss4};
    if (run_.bosses <= 4)
      award(b[run_.bosses - 1]);
    if (run_.bosses == 1) {
      award(static_cast<Badge>(static_cast<int>(Badge::Boss1Warrior) +
                               static_cast<int>(run_.heroClass)));
      if (hasBadge(Badge::Boss1Warrior) && hasBadge(Badge::Boss1Mage) &&
          hasBadge(Badge::Boss1Rogue) && hasBadge(Badge::Boss1Huntress))
        award(Badge::Boss1AllClasses);
    }
    if (run_.bosses == 3 && subClass != HeroSubClass::None) {
      const int index = static_cast<int>(subClass) - 1;
      award(
          static_cast<Badge>(static_cast<int>(Badge::Boss3Gladiator) + index));
      bool all = true;
      for (int i = 0; i < 8; ++i)
        all = all && hasBadge(static_cast<Badge>(
                         static_cast<int>(Badge::Boss3Gladiator) + i));
      if (all)
        award(Badge::Boss3AllSubclasses);
    }
  }
  validateRunThresholds();
}
void Progression::noteGold(int value) {
  run_.gold += std::max(0, value);
  totalGold_ += std::max(0, value);
  if (totalGold_ >= 1000)
    award(Badge::TreasureHunter);
  validateRunThresholds();
}
void Progression::noteItem(int level) {
  ++run_.items;
  ++totalItems_;
  if (totalItems_ >= 50)
    award(Badge::ItemCollector);
  if (level >= 3)
    award(Badge::ItemLevel1);
  if (level >= 6)
    award(Badge::ItemLevel2);
  if (level >= 9)
    award(Badge::ItemLevel3);
  if (level >= 12)
    award(Badge::ItemLevel4);
}
void Progression::noteFood() {
  ++run_.food;
  validateRunThresholds();
}
void Progression::notePotionCooked() {
  ++run_.potionsCooked;
  validateRunThresholds();
}
void Progression::notePiranha() {
  ++run_.piranhasKilled;
  if (run_.piranhasKilled >= 6)
    award(Badge::Piranhas);
}
void Progression::noteAnkh() { ++run_.ankhsUsed; }
void Progression::noteCombo(int n) {
  if (n >= 7)
    award(Badge::MasteryCombo);
}
void Progression::noteRare(int kind) {
  if (kind >= 0 && kind < 5)
    award(static_cast<Badge>(static_cast<int>(Badge::RareAlbino) + kind));
  if (hasBadge(Badge::RareAlbino) && hasBadge(Badge::RareBandit) &&
      hasBadge(Badge::RareShielded) && hasBadge(Badge::RareSenior) &&
      hasBadge(Badge::RareAcidic))
    award(Badge::Rare);
}
void Progression::noteBag(int kind) {
  if (kind >= 0 && kind < 3)
    award(static_cast<Badge>(static_cast<int>(Badge::BagSeedPouch) + kind));
  if (hasBadge(Badge::BagSeedPouch) && hasBadge(Badge::BagScrollHolder) &&
      hasBadge(Badge::BagWandHolster))
    award(Badge::AllBagsBought);
}
void Progression::notePotionIdentified() {
  award(Badge::AllPotionsIdentified);
  if (hasBadge(Badge::AllScrollsIdentified) &&
      hasBadge(Badge::AllRingsIdentified) &&
      hasBadge(Badge::AllWandsIdentified))
    award(Badge::AllItemsIdentified);
}
void Progression::noteScrollIdentified() {
  award(Badge::AllScrollsIdentified);
  if (hasBadge(Badge::AllPotionsIdentified) &&
      hasBadge(Badge::AllRingsIdentified) &&
      hasBadge(Badge::AllWandsIdentified))
    award(Badge::AllItemsIdentified);
}
void Progression::noteRingIdentified() {
  award(Badge::AllRingsIdentified);
  if (hasBadge(Badge::AllPotionsIdentified) &&
      hasBadge(Badge::AllScrollsIdentified) &&
      hasBadge(Badge::AllWandsIdentified))
    award(Badge::AllItemsIdentified);
}
void Progression::noteWandIdentified() {
  award(Badge::AllWandsIdentified);
  if (hasBadge(Badge::AllPotionsIdentified) &&
      hasBadge(Badge::AllScrollsIdentified) &&
      hasBadge(Badge::AllRingsIdentified))
    award(Badge::AllItemsIdentified);
}
void Progression::noteDeath(Badge b) {
  award(b);
  if (hasBadge(Badge::DeathFire) && hasBadge(Badge::DeathPoison) &&
      hasBadge(Badge::DeathGas) && hasBadge(Badge::DeathHunger))
    award(Badge::YASD);
}
void Progression::noteHappyEnd() {
  award(Badge::HappyEnd);
  save();
}
void Progression::validateRunThresholds() {
  const int kills[] = {10, 50, 150, 250};
  const Badge kb[] = {Badge::MonstersSlain1, Badge::MonstersSlain2,
                      Badge::MonstersSlain3, Badge::MonstersSlain4};
  const int gold[] = {100, 500, 2500, 7500};
  const Badge gb[] = {Badge::GoldCollected1, Badge::GoldCollected2,
                      Badge::GoldCollected3, Badge::GoldCollected4};
  const int levels[] = {6, 12, 18, 24};
  const Badge lb[] = {Badge::LevelReached1, Badge::LevelReached2,
                      Badge::LevelReached3, Badge::LevelReached4};
  const int strength[] = {13, 15, 17, 19};
  const Badge sb[] = {Badge::Strength1, Badge::Strength2, Badge::Strength3,
                      Badge::Strength4};
  const int food[] = {10, 20, 30, 40};
  const Badge fb[] = {Badge::Food1, Badge::Food2, Badge::Food3, Badge::Food4};
  const int cooked[] = {3, 6, 9, 12};
  const Badge cb[] = {Badge::PotionsCooked1, Badge::PotionsCooked2,
                      Badge::PotionsCooked3, Badge::PotionsCooked4};
  for (int i = 0; i < 4; ++i) {
    if (run_.kills >= kills[i])
      award(kb[i]);
    if (run_.gold >= gold[i])
      award(gb[i]);
    if (run_.maxHeroLevel >= levels[i])
      award(lb[i]);
    if (run_.maxStrength >= strength[i])
      award(sb[i]);
    if (run_.food >= food[i])
      award(fb[i]);
    if (run_.potionsCooked >= cooked[i])
      award(cb[i]);
  }
}
void Progression::insertRanking(const RankingEntry &e) {
  auto it =
      std::find_if(rankings_.begin(), rankings_.end(),
                   [&](const RankingEntry &v) { return e.score > v.score; });
  if (it == rankings_.end())
    return;
  for (auto p = rankings_.end() - 1; p > it; --p)
    *p = *(p - 1);
  *it = e;
}
void Progression::finishRun(bool won) {
  ++runs_;
  if (won) {
    ++wins_;
    run_.amuletObtained = true;
    award(Badge::Victory);
    award(static_cast<Badge>(static_cast<int>(Badge::VictoryWarrior) +
                             static_cast<int>(run_.heroClass)));
    if (hasBadge(Badge::VictoryWarrior) && hasBadge(Badge::VictoryMage) &&
        hasBadge(Badge::VictoryRogue) && hasBadge(Badge::VictoryHuntress))
      award(Badge::VictoryAllClasses);
  }
  if (run_.completedWithNoKilling)
    award(Badge::NoMonstersSlain);
  if (runs_ >= 10)
    award(Badge::GamesPlayed1);
  if (runs_ >= 100)
    award(Badge::GamesPlayed2);
  if (runs_ >= 500)
    award(Badge::GamesPlayed3);
  if (runs_ >= 2000)
    award(Badge::GamesPlayed4);
  RankingEntry e;
  e.depth = run_.depth;
  e.heroClass = static_cast<int>(run_.heroClass);
  e.kills = run_.kills;
  e.gold = run_.gold;
  e.duration = run_.duration;
  e.won = won;
  e.score = (run_.gold + run_.maxHeroLevel * run_.depth * 100) * (won ? 2 : 1);
  insertRanking(e);
  save();
}
void Progression::addJournal(JournalFeature feature, int depth) {
  for (const auto &r : journal_)
    if (r.feature == feature && r.depth == depth)
      return;
  journal_.push_back({feature, depth});
  std::sort(journal_.begin(), journal_.end(),
            [](const JournalRecord &a, const JournalRecord &b) {
              return a.depth > b.depth;
            });
}
void Progression::removeJournal(JournalFeature feature, int depth) {
  journal_.erase(std::remove_if(journal_.begin(), journal_.end(),
                                [&](const JournalRecord &r) {
                                  return r.feature == feature &&
                                         r.depth == depth;
                                }),
                 journal_.end());
}
std::string Progression::badgeName(Badge b) const {
  static const char *names[] = {"First Blood",
                                "Treasure Hunter",
                                "Deep Explorer",
                                "Monster Hunter",
                                "Item Collector",
                                "Boss Slayer",
                                "Dungeon Master",
                                "Victory",
                                "10 enemies slain",
                                "50 enemies slain",
                                "150 enemies slain",
                                "250 enemies slain",
                                "100 gold collected",
                                "500 gold collected",
                                "2500 gold collected",
                                "7500 gold collected",
                                "Level 6 reached",
                                "Level 12 reached",
                                "Level 18 reached",
                                "Level 24 reached",
                                "All potions identified",
                                "All scrolls identified",
                                "All rings identified",
                                "All wands identified",
                                "All items identified",
                                "Seed pouch acquired",
                                "Scroll holder acquired",
                                "Wand holster acquired",
                                "All bags acquired",
                                "Death by fire",
                                "Death by poison",
                                "Death by toxic gas",
                                "Death by hunger",
                                "Death by glyph",
                                "Death by falling",
                                "Yet Another Stupid Death",
                                "Goo defeated as Warrior",
                                "Goo defeated as Mage",
                                "Goo defeated as Rogue",
                                "Goo defeated as Huntress",
                                "First boss defeated",
                                "Second boss defeated",
                                "Third boss defeated",
                                "Fourth boss defeated",
                                "First boss defeated by every class",
                                "Third boss defeated as Gladiator",
                                "Third boss defeated as Berserker",
                                "Third boss defeated as Warlock",
                                "Third boss defeated as Battlemage",
                                "Third boss defeated as Freerunner",
                                "Third boss defeated as Assassin",
                                "Third boss defeated as Sniper",
                                "Third boss defeated as Warden",
                                "Third boss defeated by every subclass",
                                "Ring of Haggler",
                                "Ring of Thorns",
                                "Strength 13",
                                "Strength 15",
                                "Strength 17",
                                "Strength 19",
                                "10 meals eaten",
                                "20 meals eaten",
                                "30 meals eaten",
                                "40 meals eaten",
                                "Warrior mastery",
                                "Mage mastery",
                                "Rogue mastery",
                                "Huntress mastery",
                                "Item upgraded to +3",
                                "Item upgraded to +6",
                                "Item upgraded to +9",
                                "Item upgraded to +12",
                                "Albino rat defeated",
                                "Bandit defeated",
                                "Shielded brute defeated",
                                "Senior monk defeated",
                                "Acidic scorpion defeated",
                                "All rare enemies defeated",
                                "Victory as Warrior",
                                "Victory as Mage",
                                "Victory as Rogue",
                                "Victory as Huntress",
                                "Victory with every class",
                                "Mastery combo",
                                "3 potions cooked",
                                "6 potions cooked",
                                "9 potions cooked",
                                "12 potions cooked",
                                "No monsters slain",
                                "Grim weapon",
                                "Piranha hunter",
                                "Night hunter",
                                "10 games played",
                                "100 games played",
                                "500 games played",
                                "2000 games played",
                                "Happy end",
                                "Champion",
                                "Supporter"};
  static_assert(sizeof(names) / sizeof(*names) ==
                    static_cast<std::size_t>(Badge::Count),
                "Every badge needs a display name");
  const int index = static_cast<int>(b);
  if (index >= 0 && index < static_cast<int>(Badge::Count))
    return names[index];
  return "PIXEL DUNGEON CONSOLE EDITION achievement";
}
} // namespace pdv
