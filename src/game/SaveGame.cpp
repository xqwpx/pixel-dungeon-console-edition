#include "game/Dungeon.h"
#include "game/Progression.h"

#include "game/actors/mobs/Albino.h"
#include "game/actors/mobs/Crab.h"
#include "game/actors/mobs/Gnoll.h"
#include "game/actors/mobs/Goo.h"
#include "game/actors/mobs/Piranha.h"
#include "game/actors/mobs/Rat.h"
#include "game/actors/mobs/Skeleton.h"
#include "game/actors/mobs/Swarm.h"
#include "game/actors/mobs/Thief.h"
#include "game/actors/mobs/ZoneMob.h"
#include "game/actors/mobs/Mimic.h"
#include "game/actors/mobs/SpecialMob.h"
#include "game/actors/mobs/npcs/DungeonNPC.h"
#include "game/actors/buffs/Regeneration.h"
#include "game/actors/buffs/Amok.h"
#include "game/actors/buffs/Awareness.h"
#include "game/actors/buffs/Barkskin.h"
#include "game/actors/buffs/Bleeding.h"
#include "game/actors/buffs/Blindness.h"
#include "game/actors/buffs/Burning.h"
#include "game/actors/buffs/Charm.h"
#include "game/actors/buffs/Combo.h"
#include "game/actors/buffs/Cripple.h"
#include "game/actors/buffs/DeferredDamage.h"
#include "game/actors/buffs/Frost.h"
#include "game/actors/buffs/Fury.h"
#include "game/actors/buffs/GasesImmunity.h"
#include "game/actors/buffs/HerbalArmor.h"
#include "game/actors/buffs/HerbalHealing.h"
#include "game/actors/buffs/Invisibility.h"
#include "game/actors/buffs/Levitation.h"
#include "game/actors/buffs/Light.h"
#include "game/actors/buffs/MindVision.h"
#include "game/actors/buffs/Ooze.h"
#include "game/actors/buffs/Paralysis.h"
#include "game/actors/buffs/Poison.h"
#include "game/actors/buffs/Rage.h"
#include "game/actors/buffs/Roots.h"
#include "game/actors/buffs/Shadows.h"
#include "game/actors/buffs/Sleep.h"
#include "game/actors/buffs/Slow.h"
#include "game/actors/buffs/SnipersMark.h"
#include "game/actors/buffs/Speed.h"
#include "game/actors/buffs/Terror.h"
#include "game/actors/buffs/Vertigo.h"
#include "game/actors/buffs/Weakness.h"
#include "game/actors/blobs/Blob.h"
#include "game/items/Bag.h"
#include "game/items/Gold.h"
#include "game/items/EquipmentCatalog.h"
#include "game/items/ConsumableCatalog.h"
#include "game/items/SpecialItems.h"
#include "game/items/Heap.h"
#include "game/items/armor/Armor.h"
#include "game/items/armor/ClothArmor.h"
#include "game/items/bags/Keyring.h"
#include "game/items/bags/ScrollHolder.h"
#include "game/items/bags/SeedPouch.h"
#include "game/items/food/Food.h"
#include "game/items/food/Pasty.h"
#include "game/items/keys/GoldenKey.h"
#include "game/items/keys/IronKey.h"
#include "game/items/keys/Key.h"
#include "game/items/keys/SkeletonKey.h"
#include "game/items/potions/Potion.h"
#include "game/items/potions/PotionOfExperience.h"
#include "game/items/potions/PotionOfFrost.h"
#include "game/items/potions/PotionOfHealing.h"
#include "game/items/potions/PotionOfInvisibility.h"
#include "game/items/potions/PotionOfLevitation.h"
#include "game/items/potions/PotionOfLiquidFlame.h"
#include "game/items/potions/PotionOfMight.h"
#include "game/items/potions/PotionOfMindVision.h"
#include "game/items/potions/PotionOfParalyticGas.h"
#include "game/items/potions/PotionOfPurity.h"
#include "game/items/potions/PotionOfStrength.h"
#include "game/items/potions/PotionOfToxicGas.h"
#include "game/items/rings/Ring.h"
#include "game/items/rings/RingOfShadows.h"
#include "game/items/scrolls/Scroll.h"
#include "game/items/seeds/Seed.h"
#include "game/items/wands/Wand.h"
#include "game/items/wands/WandOfMagicMissile.h"
#include "game/items/weapon/melee/Dagger.h"
#include "game/items/weapon/melee/Knuckles.h"
#include "game/items/weapon/melee/ShortSword.h"
#include "game/items/weapon/missiles/Boomerang.h"
#include "game/items/weapon/missiles/Dart.h"
#include "game/actors/blobs/SacrificialFire.h"
#include "game/utils/Random.h"

#include <cstdio>
#include <cmath>
#include <cstring>
#include <limits>
#if defined(PD_PLATFORM_PS1)
#include "platform/ps1/PS1FileIO.h"
#endif
#include <memory>
#include <vector>

namespace pdv {
namespace {

constexpr uint32_t SaveMagic = 0x56534450u; // PDSV
constexpr uint32_t SaveVersion = 26u;
constexpr uint32_t BonesMagic = 0x534E4250u; // PBNS
bool hostIsLittleEndian(){const uint16_t probe=1;return *reinterpret_cast<const uint8_t*>(&probe)==1;}

class Writer {
public:
    template<class T> void put(T value) {
        // Save payloads are always little-endian, independently of host CPU.
        uint8_t raw[sizeof(T)]{};
        std::memcpy(raw, &value, sizeof(T));
        for (std::size_t i = 0; i < sizeof(T); ++i) data.push_back(raw[hostIsLittleEndian() ? i : sizeof(T)-1-i]);
    }
    void bytes(const void* value,std::size_t size){const uint8_t* p=static_cast<const uint8_t*>(value);data.insert(data.end(),p,p+size);}
    std::vector<uint8_t> data;
};

class Reader {
public:
    explicit Reader(const std::vector<uint8_t>& bytes):data(bytes){}
    template<class T> bool get(T& value){if(pos+sizeof(T)>data.size())return false;uint8_t raw[sizeof(T)]{};for(std::size_t i=0;i<sizeof(T);++i)raw[hostIsLittleEndian()?i:sizeof(T)-1-i]=data[pos+i];std::memcpy(&value,raw,sizeof(T));pos+=sizeof(T);return true;}
    bool bytes(void* value,std::size_t size){if(pos+size>data.size())return false;std::memcpy(value,data.data()+pos,size);pos+=size;return true;}
    bool done()const{return pos==data.size();}
private:const std::vector<uint8_t>& data;std::size_t pos=0;
};

uint32_t checksum(const uint8_t* data,std::size_t size){uint32_t hash=2166136261u;for(std::size_t i=0;i<size;++i){hash^=data[i];hash*=16777619u;}return hash;}

enum BuffCode:int32_t{BAmok=1,BAwareness,BBarkskin,BBleeding,BBlindness,BBurning,BCharm,BCombo,BCripple,BFrost,BFury,BGases,BHerbalArmor,BHerbalHealing,BInvisibility,BLevitation,BLight,BMindVision,BOoze,BParalysis,BPoison,BRage,BRoots,BShadows,BSleep,BSlow,BSnipersMark,BSpeed,BTerror,BVertigo,BWeakness,BDeferredDamage,BMarked};
int32_t buffCode(const Buff&b){
    if(dynamic_cast<const DeferredDamage*>(&b))return BDeferredDamage;
#define PDV_BUFF_CODE(type,code) if(dynamic_cast<const type*>(&b))return code
    PDV_BUFF_CODE(Shadows,BShadows);PDV_BUFF_CODE(Amok,BAmok);PDV_BUFF_CODE(Awareness,BAwareness);PDV_BUFF_CODE(Barkskin,BBarkskin);PDV_BUFF_CODE(Bleeding,BBleeding);PDV_BUFF_CODE(Blindness,BBlindness);PDV_BUFF_CODE(Burning,BBurning);PDV_BUFF_CODE(Charm,BCharm);PDV_BUFF_CODE(Combo,BCombo);PDV_BUFF_CODE(Cripple,BCripple);PDV_BUFF_CODE(Frost,BFrost);PDV_BUFF_CODE(Fury,BFury);PDV_BUFF_CODE(GasesImmunity,BGases);PDV_BUFF_CODE(HerbalArmor,BHerbalArmor);PDV_BUFF_CODE(HerbalHealing,BHerbalHealing);PDV_BUFF_CODE(Invisibility,BInvisibility);PDV_BUFF_CODE(Levitation,BLevitation);PDV_BUFF_CODE(Light,BLight);PDV_BUFF_CODE(MindVision,BMindVision);PDV_BUFF_CODE(Ooze,BOoze);PDV_BUFF_CODE(Paralysis,BParalysis);PDV_BUFF_CODE(Poison,BPoison);PDV_BUFF_CODE(Rage,BRage);PDV_BUFF_CODE(Roots,BRoots);PDV_BUFF_CODE(Sleep,BSleep);PDV_BUFF_CODE(Slow,BSlow);PDV_BUFF_CODE(SnipersMark,BSnipersMark);PDV_BUFF_CODE(Speed,BSpeed);PDV_BUFF_CODE(Terror,BTerror);PDV_BUFF_CODE(Vertigo,BVertigo);PDV_BUFF_CODE(Weakness,BWeakness);PDV_BUFF_CODE(Marked,BMarked);return 0;
#undef PDV_BUFF_CODE
}
std::unique_ptr<Buff>makeBuff(int32_t code){switch(code){case BAmok:return std::unique_ptr<Buff>(new Amok());case BAwareness:return std::unique_ptr<Buff>(new Awareness());case BBarkskin:return std::unique_ptr<Buff>(new Barkskin());case BBleeding:return std::unique_ptr<Buff>(new Bleeding());case BBlindness:return std::unique_ptr<Buff>(new Blindness());case BBurning:return std::unique_ptr<Buff>(new Burning());case BCharm:return std::unique_ptr<Buff>(new Charm());case BCombo:return std::unique_ptr<Buff>(new Combo());case BCripple:return std::unique_ptr<Buff>(new Cripple());case BFrost:return std::unique_ptr<Buff>(new Frost());case BFury:return std::unique_ptr<Buff>(new Fury());case BGases:return std::unique_ptr<Buff>(new GasesImmunity());case BHerbalArmor:return std::unique_ptr<Buff>(new HerbalArmor());case BHerbalHealing:return std::unique_ptr<Buff>(new HerbalHealing());case BInvisibility:return std::unique_ptr<Buff>(new Invisibility());case BLevitation:return std::unique_ptr<Buff>(new Levitation());case BLight:return std::unique_ptr<Buff>(new Light());case BMindVision:return std::unique_ptr<Buff>(new MindVision());case BOoze:return std::unique_ptr<Buff>(new Ooze());case BParalysis:return std::unique_ptr<Buff>(new Paralysis());case BPoison:return std::unique_ptr<Buff>(new Poison());case BRage:return std::unique_ptr<Buff>(new Rage());case BRoots:return std::unique_ptr<Buff>(new Roots());case BShadows:return std::unique_ptr<Buff>(new Shadows());case BSleep:return std::unique_ptr<Buff>(new Sleep());case BSlow:return std::unique_ptr<Buff>(new Slow());case BSnipersMark:return std::unique_ptr<Buff>(new SnipersMark());case BSpeed:return std::unique_ptr<Buff>(new Speed());case BTerror:return std::unique_ptr<Buff>(new Terror());case BVertigo:return std::unique_ptr<Buff>(new Vertigo());case BWeakness:return std::unique_ptr<Buff>(new Weakness());default:return nullptr;}}
struct SavedBuff{int32_t code=0;float cooldown=0,value=0;int32_t state=0;};
void writeBuffs(Writer&w,const Char&ch){uint32_t count=0;for(const Buff*buff:ch.activeBuffs())if(buff&&buffCode(*buff))++count;w.put(count);for(const Buff*buff:ch.activeBuffs())if(buff){const int32_t code=buffCode(*buff);if(code){w.put(code);w.put<float>(buff->cooldown());w.put<float>(buff->persistentValue());w.put<int32_t>(buff->persistentInt());}}}
bool readBuffs(Reader&r,std::vector<SavedBuff>&out){uint32_t count=0;if(!r.get(count)||count>64)return false;out.resize(count);for(SavedBuff&buff:out)if(!r.get(buff.code)||buff.code<BAmok||buff.code>BMarked||!r.get(buff.cooldown)||buff.cooldown<0.0f||buff.cooldown>10000.0f||!r.get(buff.value)||!r.get(buff.state))return false;return true;}
bool restoreBuffs(Char&ch,const std::vector<SavedBuff>&saved){for(const SavedBuff&record:saved){std::unique_ptr<Buff>buff=record.code==BDeferredDamage?std::unique_ptr<Buff>(new DeferredDamage()):(record.code==BMarked?std::unique_ptr<Buff>(new Marked()):makeBuff(record.code));if(!buff)return false;Buff*raw=ch.adoptBuff(std::move(buff));if(!raw)return false;raw->restorePersistent(record.cooldown,record.value,record.state);}return true;}

enum ItemCode : uint16_t {
    IGold=1, IFood, IPasty, ISeed, IScroll, IIronKey, IGoldenKey, ISkeletonKey,
    IKeyring, IScrollHolder, ISeedPouch, IDagger, IKnuckles, IShortSword,
    IDart, IBoomerang, IClothArmor, IRingShadows, IWandMagicMissile,
    IPotionExperience=30, IPotionFrost, IPotionHealing, IPotionInvisibility,
    IPotionLevitation, IPotionLiquidFlame, IPotionMight, IPotionMindVision,
    IPotionParalyticGas, IPotionPurity, IPotionStrength, IPotionToxicGas,
    ICatalogMelee=60,ICatalogMissile,ICatalogArmor,ICatalogRing,ICatalogWand,ICatalogSeed,IMysteryMeat,ICatalogMisc,ISpecialItem,IClassArmor,IPreparedFood,IWandHolster,IQuestPickaxe
};

uint16_t itemCode(const Item& item){
#define PDV_ITEM_CODE(type,code) if(dynamic_cast<const type*>(&item))return code
    PDV_ITEM_CODE(Pasty,IPasty);PDV_ITEM_CODE(MysteryMeat,IMysteryMeat);PDV_ITEM_CODE(PreparedFood,IPreparedFood);PDV_ITEM_CODE(CatalogSeed,ICatalogSeed);PDV_ITEM_CODE(Food,IFood); PDV_ITEM_CODE(Gold,IGold); PDV_ITEM_CODE(Seed,ISeed);
    PDV_ITEM_CODE(IronKey,IIronKey); PDV_ITEM_CODE(GoldenKey,IGoldenKey); PDV_ITEM_CODE(SkeletonKey,ISkeletonKey);
    PDV_ITEM_CODE(Keyring,IKeyring); PDV_ITEM_CODE(ScrollHolder,IScrollHolder); PDV_ITEM_CODE(SeedPouch,ISeedPouch);
    PDV_ITEM_CODE(Dagger,IDagger); PDV_ITEM_CODE(Knuckles,IKnuckles); PDV_ITEM_CODE(ShortSword,IShortSword);
    PDV_ITEM_CODE(Dart,IDart); PDV_ITEM_CODE(Boomerang,IBoomerang); PDV_ITEM_CODE(ClothArmor,IClothArmor);
    PDV_ITEM_CODE(RingOfShadows,IRingShadows); PDV_ITEM_CODE(WandOfMagicMissile,IWandMagicMissile);
    PDV_ITEM_CODE(CatalogMelee,ICatalogMelee);PDV_ITEM_CODE(CatalogMissile,ICatalogMissile);PDV_ITEM_CODE(CatalogArmor,ICatalogArmor);PDV_ITEM_CODE(CatalogRing,ICatalogRing);PDV_ITEM_CODE(CatalogWand,ICatalogWand);
    PDV_ITEM_CODE(CatalogMisc,ICatalogMisc);PDV_ITEM_CODE(SpecialItem,ISpecialItem);PDV_ITEM_CODE(ClassArmor,IClassArmor);PDV_ITEM_CODE(WandHolster,IWandHolster);PDV_ITEM_CODE(QuestPickaxe,IQuestPickaxe);
    PDV_ITEM_CODE(PotionOfExperience,IPotionExperience); PDV_ITEM_CODE(PotionOfFrost,IPotionFrost);
    PDV_ITEM_CODE(PotionOfHealing,IPotionHealing); PDV_ITEM_CODE(PotionOfInvisibility,IPotionInvisibility);
    PDV_ITEM_CODE(PotionOfLevitation,IPotionLevitation); PDV_ITEM_CODE(PotionOfLiquidFlame,IPotionLiquidFlame);
    PDV_ITEM_CODE(PotionOfMight,IPotionMight); PDV_ITEM_CODE(PotionOfMindVision,IPotionMindVision);
    PDV_ITEM_CODE(PotionOfParalyticGas,IPotionParalyticGas); PDV_ITEM_CODE(PotionOfPurity,IPotionPurity);
    PDV_ITEM_CODE(PotionOfStrength,IPotionStrength); PDV_ITEM_CODE(PotionOfToxicGas,IPotionToxicGas);
    if(dynamic_cast<const Scroll*>(&item))return IScroll;
#undef PDV_ITEM_CODE
    return 0;
}

std::unique_ptr<Item> makeItem(uint16_t code,int extra){
    switch(code){
    case IGold:return std::unique_ptr<Item>(new Gold()); case IFood:return std::unique_ptr<Item>(new Food()); case IPasty:return std::unique_ptr<Item>(new Pasty()); case ISeed:return std::unique_ptr<Item>(new Seed());
    case IScroll:return std::unique_ptr<Item>(new Scroll(static_cast<ScrollKind>(extra)));
    case IIronKey:return std::unique_ptr<Item>(new IronKey(extra)); case IGoldenKey:return std::unique_ptr<Item>(new GoldenKey(extra)); case ISkeletonKey:return std::unique_ptr<Item>(new SkeletonKey(extra));
    case IKeyring:return std::unique_ptr<Item>(new Keyring()); case IScrollHolder:return std::unique_ptr<Item>(new ScrollHolder()); case ISeedPouch:return std::unique_ptr<Item>(new SeedPouch());
    case IDagger:return std::unique_ptr<Item>(new Dagger()); case IKnuckles:return std::unique_ptr<Item>(new Knuckles()); case IShortSword:return std::unique_ptr<Item>(new ShortSword());
    case IDart:return std::unique_ptr<Item>(new Dart()); case IBoomerang:return std::unique_ptr<Item>(new Boomerang()); case IClothArmor:return std::unique_ptr<Item>(new ClothArmor());
    case IRingShadows:return std::unique_ptr<Item>(new RingOfShadows()); case IWandMagicMissile:return std::unique_ptr<Item>(new WandOfMagicMissile());
    case IPotionExperience:return std::unique_ptr<Item>(new PotionOfExperience()); case IPotionFrost:return std::unique_ptr<Item>(new PotionOfFrost()); case IPotionHealing:return std::unique_ptr<Item>(new PotionOfHealing());
    case IPotionInvisibility:return std::unique_ptr<Item>(new PotionOfInvisibility()); case IPotionLevitation:return std::unique_ptr<Item>(new PotionOfLevitation()); case IPotionLiquidFlame:return std::unique_ptr<Item>(new PotionOfLiquidFlame());
    case IPotionMight:return std::unique_ptr<Item>(new PotionOfMight()); case IPotionMindVision:return std::unique_ptr<Item>(new PotionOfMindVision()); case IPotionParalyticGas:return std::unique_ptr<Item>(new PotionOfParalyticGas());
    case IPotionPurity:return std::unique_ptr<Item>(new PotionOfPurity()); case IPotionStrength:return std::unique_ptr<Item>(new PotionOfStrength()); case IPotionToxicGas:return std::unique_ptr<Item>(new PotionOfToxicGas());
    case ICatalogMelee:return std::unique_ptr<Item>(new CatalogMelee(static_cast<MeleeKind>(extra)));case ICatalogMissile:return std::unique_ptr<Item>(new CatalogMissile(static_cast<MissileKind>(extra)));case ICatalogArmor:return std::unique_ptr<Item>(new CatalogArmor(static_cast<ArmorKind>(extra)));case ICatalogRing:return std::unique_ptr<Item>(new CatalogRing(static_cast<RingKind>(extra)));case ICatalogWand:return std::unique_ptr<Item>(new CatalogWand(static_cast<WandKind>(extra)));
    case ICatalogSeed:return std::unique_ptr<Item>(new CatalogSeed(static_cast<SeedKind>(extra)));case IMysteryMeat:return std::unique_ptr<Item>(new MysteryMeat());case ICatalogMisc:return std::unique_ptr<Item>(new CatalogMisc(static_cast<MiscKind>(extra)));
    case ISpecialItem:return std::unique_ptr<Item>(new SpecialItem(static_cast<SpecialKind>(extra)));case IClassArmor:return std::unique_ptr<Item>(new ClassArmor(static_cast<ClassArmorKind>(extra)));case IPreparedFood:return std::unique_ptr<Item>(new PreparedFood(static_cast<PreparedFoodKind>(extra)));case IWandHolster:return std::unique_ptr<Item>(new WandHolster());case IQuestPickaxe:return std::unique_ptr<Item>(new QuestPickaxe());
    default:return nullptr;
    }
}

bool writeItem(Writer& w,const Item& item){
    const uint16_t code=itemCode(item); if(code==0)return false;
    int32_t extra=0;
    if(const Scroll* scroll=dynamic_cast<const Scroll*>(&item))extra=static_cast<int32_t>(scroll->kind());
    else if(const Key* key=dynamic_cast<const Key*>(&item))extra=key->depth();
    else if(const CatalogMelee* value=dynamic_cast<const CatalogMelee*>(&item))extra=static_cast<int32_t>(value->kind());
    else if(const CatalogMissile* value=dynamic_cast<const CatalogMissile*>(&item))extra=static_cast<int32_t>(value->kind());
    else if(const CatalogArmor* value=dynamic_cast<const CatalogArmor*>(&item))extra=static_cast<int32_t>(value->kind());
    else if(const CatalogRing* value=dynamic_cast<const CatalogRing*>(&item))extra=static_cast<int32_t>(value->kind());
    else if(const CatalogWand* value=dynamic_cast<const CatalogWand*>(&item))extra=static_cast<int32_t>(value->kind());
    else if(const CatalogSeed* value=dynamic_cast<const CatalogSeed*>(&item))extra=static_cast<int32_t>(value->kind());
    else if(const CatalogMisc* value=dynamic_cast<const CatalogMisc*>(&item))extra=static_cast<int32_t>(value->kind());
    else if(const SpecialItem* value=dynamic_cast<const SpecialItem*>(&item))extra=static_cast<int32_t>(value->kind());
    else if(const ClassArmor* value=dynamic_cast<const ClassArmor*>(&item))extra=static_cast<int32_t>(value->kind());
    else if(const PreparedFood* value=dynamic_cast<const PreparedFood*>(&item))extra=static_cast<int32_t>(value->kind());
    w.put(code);w.put(extra);w.put<int32_t>(item.quantity());w.put<int32_t>(item.level());
    uint8_t flags=(item.isIdentified()?1:0)|(item.cursed()?2:0)|(item.cursedKnown()?4:0);w.put(flags);
int32_t special=0;if(const Wand* wand=dynamic_cast<const Wand*>(&item))special=wand->curCharges();else if(const QuestPickaxe*pick=dynamic_cast<const QuestPickaxe*>(&item))special=pick->bloodStained()?1:0;else if(const Weapon* weapon=dynamic_cast<const Weapon*>(&item))special=weapon->enchantmentId()|(static_cast<int32_t>(weapon->imbue())<<8);else if(const Armor* armor=dynamic_cast<const Armor*>(&item))special=armor->glyphId();else if(const SpecialItem* value=dynamic_cast<const SpecialItem*>(&item))special=value->state();w.put(special);w.put<int32_t>(item.durability());
    const Bag* bag=dynamic_cast<const Bag*>(&item);uint32_t count=bag?static_cast<uint32_t>(bag->items().size()):0;w.put(count);
    if(bag)for(const auto& child:bag->items())if(!child||!writeItem(w,*child))return false;
    return true;
}

std::unique_ptr<Item> readItem(Reader& r,uint32_t version){
    uint16_t code=0;int32_t extra=0,quantity=0,level=0,special=0,durability=-1;uint8_t flags=0;uint32_t count=0;
    if(!r.get(code)||!r.get(extra)||!r.get(quantity)||!r.get(level)||!r.get(flags)||!r.get(special)||(version>=13u&&!r.get(durability))||!r.get(count))return nullptr;
    std::unique_ptr<Item> item=makeItem(code,extra);if(!item)return nullptr;item->quantity(quantity);item->level(level);item->setCursed((flags&2)!=0);item->setCursedKnown((flags&4)!=0);if(flags&1)item->identify();
if(version>=13u)item->restoreDurability(durability);if(Wand* wand=dynamic_cast<Wand*>(item.get()))wand->restoreCharges(special);else if(QuestPickaxe*pick=dynamic_cast<QuestPickaxe*>(item.get()))pick->restoreBloodStained(special!=0);else if(Weapon* weapon=dynamic_cast<Weapon*>(item.get())){if(special<0||(special&~0xFFFF)!=0||((special>>8)&0xFF)>2)return nullptr;const int enchant=special&0xFF;if(enchant>11)return nullptr;if(enchant)weapon->enchant(static_cast<EnchantmentKind>(enchant));weapon->imbue(static_cast<Weapon::Imbue>((special>>8)&0xFF));}else if(Armor* armor=dynamic_cast<Armor*>(item.get())){if(special<0||special>11)return nullptr;if(special)armor->inscribe(static_cast<GlyphKind>(special));}else if(SpecialItem* value=dynamic_cast<SpecialItem*>(item.get()))value->restoreState(special);
    Bag* bag=dynamic_cast<Bag*>(item.get());if(count&&!bag)return nullptr;for(uint32_t i=0;i<count;++i){std::unique_ptr<Item> child=readItem(r,version);if(!child||!bag->collect(std::move(child)))return nullptr;}
    return item;
}

enum MobCode:uint8_t{MRat=1,MAlbino,MGnoll,MSwarm,MCrab,MSkeleton,MThief,MGoo,MPiranha,MZone,MZoneBoss,MNPC,MMimic,MSpecial};
uint8_t mobCode(const Mob&mob){
    if(dynamic_cast<const Mimic*>(&mob))return MMimic;if(dynamic_cast<const SpecialMob*>(&mob))return MSpecial;if(dynamic_cast<const Albino*>(&mob))return MAlbino;if(dynamic_cast<const Rat*>(&mob))return MRat;if(dynamic_cast<const Gnoll*>(&mob))return MGnoll;if(dynamic_cast<const Swarm*>(&mob))return MSwarm;if(dynamic_cast<const Crab*>(&mob))return MCrab;if(dynamic_cast<const Skeleton*>(&mob))return MSkeleton;if(dynamic_cast<const Thief*>(&mob))return MThief;if(dynamic_cast<const Goo*>(&mob))return MGoo;if(dynamic_cast<const Piranha*>(&mob))return MPiranha;if(dynamic_cast<const ZoneMob*>(&mob))return MZone;if(dynamic_cast<const ZoneBoss*>(&mob))return MZoneBoss;if(dynamic_cast<const DungeonNPC*>(&mob))return MNPC;return 0;
}
std::unique_ptr<Mob>makeMob(uint8_t code,int depth,int subtype){switch(code){case MSpecial:return std::unique_ptr<Mob>(new SpecialMob(static_cast<SpecialMobKind>(subtype),depth,false));case MMimic:{std::unique_ptr<Mimic>m(new Mimic());m->adjustStats(subtype);return std::move(m);}case MRat:return std::unique_ptr<Mob>(new Rat());case MAlbino:return std::unique_ptr<Mob>(new Albino());case MGnoll:return std::unique_ptr<Mob>(new Gnoll());case MSwarm:return std::unique_ptr<Mob>(new Swarm());case MCrab:return std::unique_ptr<Mob>(new Crab());case MSkeleton:return std::unique_ptr<Mob>(new Skeleton());case MThief:return std::unique_ptr<Mob>(new Thief(subtype==1));case MGoo:return std::unique_ptr<Mob>(new Goo());case MPiranha:return std::unique_ptr<Mob>(new Piranha(depth));case MZone:if(subtype==static_cast<int>(ZoneMobKind::Bandit))return std::unique_ptr<Mob>(new Thief(true));return std::unique_ptr<Mob>(new ZoneMob(static_cast<ZoneMobKind>(subtype)));case MZoneBoss:return std::unique_ptr<Mob>(new ZoneBoss(static_cast<ZoneBossKind>(subtype)));case MNPC:return std::unique_ptr<Mob>(new DungeonNPC(static_cast<NPCKind>(subtype)));default:return nullptr;}}

} // namespace

bool Dungeon::saveExists(const std::string& path){FILE* file=std::fopen(path.c_str(),"rb");if(!file)file=std::fopen((path+".bak").c_str(),"rb");if(!file)return false;std::fclose(file);return true;}

bool Dungeon::leaveBones(const std::string&path)const{
 const Item*choices[]={hero_.belongings().weapon(),hero_.belongings().armor(),hero_.belongings().ring1(),hero_.belongings().ring2()};
 const Item*chosen=choices[static_cast<std::size_t>(Random::Int(4))];std::unique_ptr<Item>item=chosen?chosen->clone():std::unique_ptr<Item>(new Gold(std::max(1,Random::IntRange(1,std::max(1,gold_)))));
 if(!item)return false;Writer w;w.put(BonesMagic);w.put(SaveVersion);w.put<int32_t>(depth_);if(!writeItem(w,*item))return false;w.put(checksum(w.data.data(),w.data.size()));const std::string temp=path+".tmp";FILE*file=std::fopen(temp.c_str(),"wb");if(!file)return false;const bool ok=std::fwrite(w.data.data(),1,w.data.size(),file)==w.data.size()&&std::fflush(file)==0;std::fclose(file);if(!ok){std::remove(temp.c_str());return false;}std::remove(path.c_str());if(std::rename(temp.c_str(),path.c_str())!=0){std::remove(temp.c_str());return false;}return true;
}

std::unique_ptr<Item>Dungeon::takeBones(int depth,const std::string&path)const{
 FILE*file=std::fopen(path.c_str(),"rb");if(!file)return nullptr;std::fseek(file,0,SEEK_END);const long size=std::ftell(file);std::rewind(file);if(size<20||size>65536){std::fclose(file);return nullptr;}std::vector<uint8_t>bytes(static_cast<std::size_t>(size));const bool ok=std::fread(bytes.data(),1,bytes.size(),file)==bytes.size();std::fclose(file);if(!ok)return nullptr;uint32_t stored=0;std::memcpy(&stored,bytes.data()+bytes.size()-4,4);if(stored!=checksum(bytes.data(),bytes.size()-4))return nullptr;bytes.resize(bytes.size()-4);Reader r(bytes);uint32_t magic=0,version=0;int32_t savedDepth=0;if(!r.get(magic)||!r.get(version)||!r.get(savedDepth)||magic!=BonesMagic||version<13u||version>SaveVersion||savedDepth!=depth)return nullptr;std::unique_ptr<Item>item=readItem(r,version);if(!item||!r.done())return nullptr;std::remove(path.c_str());if(!item->stackable()){item->setCursed(true);item->setCursedKnown(true);if(item->isUpgradable()){const int cap=(depth-1)*3/5+1;if(item->level()>cap)item->degrade(item->level()-cap);}}return item;
}

bool Dungeon::save(const std::string& path)const{
    if(!started_) return false;
    if(!hero_.isAlive()||won_){std::remove(path.c_str());std::remove((path+".tmp").c_str());std::remove((path+".bak").c_str());return false;}
    Writer w;w.put(SaveMagic);w.put(SaveVersion);w.put(runSeed_);w.put<int32_t>(depth_);w.put<int32_t>(gold_);w.put<int32_t>(potionOfStrength_);w.put<int32_t>(scrollsOfUpgrade_);w.put<int32_t>(scrollsOfEnchantment_);w.put<uint16_t>(Potion::knownMask());w.put<uint16_t>(Scroll::knownMask());w.put<uint16_t>(Ring::knownMask());w.put<uint16_t>(Wand::knownMask());w.put<uint8_t>(won_?1:0);for(int state:questStates_)w.put<int32_t>(state);w.put<uint8_t>(blacksmithSpawned_?1:0);w.put<uint8_t>(blacksmithQuestKind_);w.put<uint8_t>(questNpcSpawnMask_);const RunStatistics&stats=Progression::instance().run();w.put<int32_t>(stats.depth);w.put<int32_t>(stats.kills);w.put<int32_t>(stats.gold);w.put<int32_t>(stats.items);w.put<int32_t>(stats.food);w.put<int32_t>(stats.bosses);w.put<int32_t>(stats.potionsCooked);w.put<int32_t>(stats.piranhasKilled);w.put<int32_t>(stats.nightHunt);w.put<int32_t>(stats.ankhsUsed);w.put<int32_t>(stats.maxHeroLevel);w.put<int32_t>(stats.maxStrength);w.put<float>(stats.duration);w.put<uint8_t>(stats.qualifiedForNoKilling?1:0);w.put<uint8_t>(stats.completedWithNoKilling?1:0);w.put<uint8_t>(stats.amuletObtained?1:0);const auto&journal=Progression::instance().journal();w.put<uint32_t>(static_cast<uint32_t>(journal.size()));for(const JournalRecord&record:journal){w.put<int32_t>(static_cast<int>(record.feature));w.put<int32_t>(record.depth);}
    w.put<int32_t>(static_cast<int>(hero_.heroClass()));w.put<int32_t>(static_cast<int>(hero_.subClass()));w.put<int32_t>(hero_.pos);w.put<int32_t>(hero_.HP);w.put<int32_t>(hero_.HT);w.put<int32_t>(hero_.baseStrength());w.put<int32_t>(hero_.level());w.put<int32_t>(hero_.exp());w.put<float>(hunger_.level());w.put<int32_t>(hero_.hpUpgrades());w.put<int32_t>(hero_.strengthUpgrades());w.put<int32_t>(hero_.enduranceUpgrades());w.put<int32_t>(hero_.luckUpgrades());
    writeBuffs(w,hero_);
    for(const auto&reward:ghostRewards_){w.put<uint8_t>(reward?1:0);if(reward&&!writeItem(w,*reward))return false;}
    w.put<int32_t>(level_.entrance());w.put<int32_t>(level_.exit());w.put<int32_t>(static_cast<int>(level_.feeling()));w.put<uint8_t>(level_.weakFloorCreated()?1:0);w.put<uint8_t>(level_.bossArenaActivated()?1:0);w.bytes(level_.map().data(),sizeof(int)*Level::Length);w.bytes(level_.visited().data(),sizeof(bool)*Level::Length);w.bytes(level_.mapped().data(),sizeof(bool)*Level::Length);w.put<uint32_t>(static_cast<uint32_t>(level_.plants().size()));for(const auto& plant:level_.plants()){w.put<int32_t>(plant.first);w.put<int32_t>(static_cast<int>(plant.second));}w.put<uint32_t>(static_cast<uint32_t>(level_.blobs().size()));for(const auto& blob:level_.blobs()){if(!blob)return false;w.put<int32_t>(static_cast<int>(blob->kind()));uint32_t cells=0;for(int i=0;i<Level::Length;++i)if(blob->valueAt(i)>0)++cells;w.put<uint32_t>(cells);for(int i=0;i<Level::Length;++i)if(blob->valueAt(i)>0){w.put<int32_t>(i);w.put<int32_t>(blob->valueAt(i));}}
    uint32_t mobCount=static_cast<uint32_t>(level_.mobs().size());w.put(mobCount);for(const auto&mob:level_.mobs()){
        if(!mob)return false;uint8_t code=mobCode(*mob);if(!code)return false;int32_t subtype=0;
        if(const ZoneMob*z=dynamic_cast<const ZoneMob*>(mob.get()))subtype=static_cast<int32_t>(z->kind());else if(const ZoneBoss*z=dynamic_cast<const ZoneBoss*>(mob.get()))subtype=static_cast<int32_t>(z->kind());else if(const DungeonNPC*z=dynamic_cast<const DungeonNPC*>(mob.get()))subtype=static_cast<int32_t>(z->kind());else if(const Mimic*z=dynamic_cast<const Mimic*>(mob.get()))subtype=z->level();else if(const SpecialMob*z=dynamic_cast<const SpecialMob*>(mob.get()))subtype=static_cast<int32_t>(z->kind());else if(const Thief*z=dynamic_cast<const Thief*>(mob.get()))subtype=z->isBandit()?1:0;
        w.put(code);w.put(subtype);w.put<int32_t>(mob->pos);w.put<int32_t>(mob->HP);w.put<int32_t>(mob->HT);w.put<int32_t>(static_cast<int>(mob->state()));const DungeonNPC*npc=dynamic_cast<const DungeonNPC*>(mob.get());w.put<uint8_t>(npc&&npc->corpseVisible()?1:0);writeBuffs(w,*mob);
        uint8_t bossFlags=0;int32_t bossTimer=0;if(const Goo*goo=dynamic_cast<const Goo*>(mob.get()))bossFlags|=goo->pumpedUp()?1u:0u;if(const ZoneBoss*boss=dynamic_cast<const ZoneBoss*>(mob.get())){bossFlags|=boss->sealed()?2u:0u;bossFlags|=boss->summonsSpawned()?4u:0u;bossTimer=boss->jumpDelay();}w.put(bossFlags);w.put(bossTimer);
        const Mimic*mimic=dynamic_cast<const Mimic*>(mob.get());const SpecialMob*special=dynamic_cast<const SpecialMob*>(mob.get());const Thief*thief=dynamic_cast<const Thief*>(mob.get());const uint32_t lootCount=mimic?static_cast<uint32_t>(mimic->items().size()):(special&&special->weapon()?1u:(thief&&thief->stolenItem()?1u:0u));w.put(lootCount);if(mimic){for(const auto&item:mimic->items())if(!item||!writeItem(w,*item))return false;}else if(special&&special->weapon()){if(!writeItem(w,*special->weapon()))return false;}else if(thief&&thief->stolenItem()&&!writeItem(w,*thief->stolenItem()))return false;const ZoneMob*zone=dynamic_cast<const ZoneMob*>(mob.get());w.put<int32_t>(special?special->depth():(zone?zone->persistentState():mob->persistentState()));
    }
    uint32_t heapCount=static_cast<uint32_t>(level_.heaps().size());w.put(heapCount);for(const auto& entry:level_.heaps()){const Heap& heap=*entry.second;w.put<int32_t>(heap.pos);w.put<int32_t>(static_cast<int>(heap.type));w.put<uint32_t>(static_cast<uint32_t>(heap.items().size()));for(const auto& item:heap.items())if(!item||!writeItem(w,*item))return false;}
    const Belongings& b=hero_.belongings();uint32_t backpackCount=static_cast<uint32_t>(b.backpack().items().size());w.put(backpackCount);for(const auto& item:b.backpack().items())if(!item||!writeItem(w,*item))return false;
    const Item* equipped[]={b.weapon(),b.armor(),b.ring1(),b.ring2()};for(const Item* item:equipped){w.put<uint8_t>(item?1:0);if(item&&!writeItem(w,*item))return false;}w.put<int32_t>(static_cast<int>(hero_.primaryQuickSlot().kind()));const Item*quickItem=hero_.primaryQuickSlot().select(hero_);int32_t quickIndex=-1;const std::vector<Item*>allItems=b.allItems();for(std::size_t i=0;i<allItems.size();++i)if(allItems[i]==quickItem){quickIndex=static_cast<int32_t>(i);break;}w.put(quickIndex);
    const uint32_t sum=checksum(w.data.data(),w.data.size());w.put(sum);const std::string temp=path+".tmp",backup=path+".bak";FILE* file=std::fopen(temp.c_str(),"wb");if(!file)return false;const bool ok=std::fwrite(w.data.data(),1,w.data.size(),file)==w.data.size()&&std::fflush(file)==0;std::fclose(file);if(!ok){std::remove(temp.c_str());return false;}std::remove(backup.c_str());const bool hadOld=std::rename(path.c_str(),backup.c_str())==0;if(std::rename(temp.c_str(),path.c_str())!=0){if(hadOld)std::rename(backup.c_str(),path.c_str());std::remove(temp.c_str());return false;}std::remove(backup.c_str());return true;
}

bool Dungeon::load(const std::string& path){
    FILE* file=std::fopen(path.c_str(),"rb");if(!file)file=std::fopen((path+".bak").c_str(),"rb");if(!file)return false;std::fseek(file,0,SEEK_END);long size=std::ftell(file);std::rewind(file);if(size<16||size>1024*1024){std::fclose(file);return false;}std::vector<uint8_t> bytes(static_cast<std::size_t>(size));const bool readOk=std::fread(bytes.data(),1,bytes.size(),file)==bytes.size();std::fclose(file);if(!readOk)return false;uint32_t stored=0;std::memcpy(&stored,bytes.data()+bytes.size()-sizeof(stored),sizeof(stored));if(stored!=checksum(bytes.data(),bytes.size()-sizeof(stored)))return false;bytes.resize(bytes.size()-sizeof(stored));Reader r(bytes);
    uint32_t magic=0,version=0,seed=0;uint16_t potionKnown=0,scrollKnown=0,ringKnown=0,wandKnown=0;uint8_t won=0,blacksmithSpawned=0,blacksmithQuestKind=0,questNpcSpawnMask=0;std::array<int,4>savedQuests{};int32_t depth=0,gold=0,posCount=0,souCount=0,soeCount=0,heroClass=0,heroSubClass=0,heroPos=0,hp=0,ht=0,strength=0,heroLevel=0,heroExp=0,hpUpgrades=0,strengthUpgrades=0,enduranceUpgrades=0,luckUpgrades=0;float hungerLevel=0;
    if(!r.get(magic)||!r.get(version)||magic!=SaveMagic||version<5u||version>SaveVersion||!r.get(seed)||!r.get(depth)||depth<1||depth>26||!r.get(gold)||!r.get(posCount)||!r.get(souCount)||!r.get(soeCount)||!r.get(potionKnown)||!r.get(scrollKnown)||(version>=19u&&(!r.get(ringKnown)||!r.get(wandKnown)))||!r.get(won))return false;
    for(int&i:savedQuests)if(!r.get(i))return false;if((version>=22u&&(!r.get(blacksmithSpawned)||blacksmithSpawned>1))||(version>=24u&&(!r.get(blacksmithQuestKind)||blacksmithQuestKind>2))||(version>=23u&&(!r.get(questNpcSpawnMask)||questNpcSpawnMask>0x07u)))return false;RunStatistics savedStats;std::vector<JournalRecord>savedJournal;if(version>=17u){uint8_t noKill=0,completedNoKill=0,amulet=0;uint32_t journalCount=0;if(!r.get(savedStats.depth)||!r.get(savedStats.kills)||!r.get(savedStats.gold)||!r.get(savedStats.items)||!r.get(savedStats.food)||!r.get(savedStats.bosses)||!r.get(savedStats.potionsCooked)||!r.get(savedStats.piranhasKilled)||!r.get(savedStats.nightHunt)||!r.get(savedStats.ankhsUsed)||!r.get(savedStats.maxHeroLevel)||!r.get(savedStats.maxStrength)||!r.get(savedStats.duration)||!std::isfinite(savedStats.duration)||savedStats.duration<0.0f||!r.get(noKill)||noKill>1||(version>=20u&&(!r.get(completedNoKill)||completedNoKill>1))||!r.get(amulet)||amulet>1||!r.get(journalCount)||journalCount>32)return false;savedStats.qualifiedForNoKilling=noKill!=0;savedStats.completedWithNoKilling=completedNoKill!=0;savedStats.amuletObtained=amulet!=0;for(uint32_t i=0;i<journalCount;++i){int32_t feature=0,recordDepth=0;if(!r.get(feature)||feature<0||feature>10||!r.get(recordDepth)||recordDepth<1||recordDepth>26)return false;JournalRecord record;record.feature=static_cast<JournalFeature>(feature);record.depth=recordDepth;savedJournal.push_back(record);}}
    if(!r.get(heroClass)||heroClass<0||heroClass>3)return false;
    if(version>=8u&&(!r.get(heroSubClass)||heroSubClass<0||heroSubClass>8))return false;
    if(!r.get(heroPos)||!r.get(hp)||!r.get(ht)||!r.get(strength)||!r.get(heroLevel)||!r.get(heroExp)||!r.get(hungerLevel)||!std::isfinite(hungerLevel))return false;
    if(version>=7u&&(!r.get(hpUpgrades)||!r.get(strengthUpgrades)||!r.get(enduranceUpgrades)))return false;
    if(version>=26u&&!r.get(luckUpgrades))return false;
    std::vector<SavedBuff>savedHeroBuffs;if(version>=11u&&!readBuffs(r,savedHeroBuffs))return false;
    std::array<std::unique_ptr<Item>,2> savedGhostRewards{};
    if(version>=25u)for(auto&reward:savedGhostRewards){uint8_t present=0;if(!r.get(present)||present>1)return false;if(present){reward=readItem(r,version);if(!reward)return false;}}
    if(!startNew(static_cast<HeroClass>(heroClass),seed)) return false;ghostRewards_=std::move(savedGhostRewards);if(version>=17u){savedStats.heroClass=static_cast<HeroClass>(heroClass);Progression::instance().restoreRun(savedStats,savedJournal);}
    Potion::restoreKnownMask(potionKnown);Scroll::restoreKnownMask(scrollKnown);Ring::restoreKnownMask(ringKnown);Wand::restoreKnownMask(wandKnown);
    scheduler_.clear();depth_=depth;runSeed_=seed;gold_=gold;won_=won!=0;questStates_=savedQuests;blacksmithSpawned_=blacksmithSpawned!=0||(version<22u&&savedQuests[2]!=0);blacksmithQuestKind_=blacksmithQuestKind;if(version<24u)blacksmithQuestKind_=savedQuests[2]==601?1:(savedQuests[2]==602?2:0);questNpcSpawnMask_=questNpcSpawnMask;if(version<23u){if(savedQuests[0]!=0)questNpcSpawnMask_|=1u;if(savedQuests[1]!=0)questNpcSpawnMask_|=2u;if(savedQuests[3]!=0)questNpcSpawnMask_|=4u;}potionOfStrength_=posCount;scrollsOfUpgrade_=souCount;scrollsOfEnchantment_=soeCount;
    const uint32_t floorSeed=runSeed_^(0x9E3779B9u*static_cast<uint32_t>(depth_));if(depth_!=1&&!level_.create(depth_,floorSeed))return false;
    int32_t entrance=0,exit=0,feeling=0;uint8_t weakFloor=0,bossArena=0;if(!r.get(entrance)||!r.get(exit)||!r.get(feeling)||(version>=17u&&!r.get(weakFloor))||(version>=21u&&(!r.get(bossArena)||bossArena>1))||!r.bytes(level_.mapMutable().data(),sizeof(int)*Level::Length)||!r.bytes(level_.visitedMutable().data(),sizeof(bool)*Level::Length)||!r.bytes(level_.mappedMutable().data(),sizeof(bool)*Level::Length))return false;level_.setEntrance(entrance);level_.setExit(exit);level_.restoreWeakFloorCreated(weakFloor!=0);level_.restoreBossArenaActivated(bossArena!=0);level_.buildFlagMaps();level_.cleanWalls();level_.clearMobs();level_.clearHeaps();level_.clearPlants();level_.clearBlobs();if(version>=9u){uint32_t plantCount=0;if(!r.get(plantCount)||plantCount>Level::Length)return false;for(uint32_t i=0;i<plantCount;++i){int32_t plantPos=0,plantKind=0;if(!r.get(plantPos)||!r.get(plantKind)||plantKind<0||plantKind>7||!level_.plant(static_cast<SeedKind>(plantKind),plantPos))return false;}}if(version>=10u){uint32_t blobCount=0;if(!r.get(blobCount)||blobCount>12)return false;for(uint32_t i=0;i<blobCount;++i){int32_t kind=0;uint32_t cells=0;if(!r.get(kind)||kind<0||kind>11||!r.get(cells)||cells>Level::Length)return false;for(uint32_t j=0;j<cells;++j){int32_t pos=0,value=0;if(!r.get(pos)||!r.get(value)||!level_.inBoundsCell(pos)||value<=0)return false;if(!level_.seedBlob(static_cast<BlobKind>(kind),pos,value))return false;}}}
uint32_t mobCount=0;if(!r.get(mobCount)||mobCount>128)return false;for(uint32_t i=0;i<mobCount;++i){
    uint8_t code=0,corpse=0,bossFlags=0;int32_t subtype=0,mobPos=0,mobHp=0,mobHt=0,state=0,bossTimer=0;
    if(!r.get(code)||!r.get(subtype)||!r.get(mobPos)||!r.get(mobHp)||!r.get(mobHt)||!r.get(state)||(version>=6u&&!r.get(corpse))||state<0||state>4||(code==MZone&&(subtype<0||subtype>19))||(code==MZoneBoss&&(subtype<0||subtype>3))||(code==MNPC&&(subtype<0||subtype>6))||(code==MMimic&&(subtype<1||subtype>25))||(code==MSpecial&&(subtype<0||subtype>4)))return false;
    std::vector<SavedBuff>savedMobBuffs;if(version>=11u&&!readBuffs(r,savedMobBuffs))return false;if(version>=12u&&(!r.get(bossFlags)||!r.get(bossTimer)||bossTimer<0||bossTimer>100))return false;
    uint32_t lootCount=0;if(version>=14u&&(!r.get(lootCount)||lootCount>64))return false;std::vector<std::unique_ptr<Item>>loot;for(uint32_t n=0;n<lootCount;++n){std::unique_ptr<Item>item=readItem(r,version);if(!item)return false;loot.push_back(std::move(item));}int32_t mobSpecial=0;if(version>=15u&&(!r.get(mobSpecial)||mobSpecial < -100||mobSpecial>1000))return false;
    std::unique_ptr<Mob>mob=makeMob(code,depth_,subtype);if(!mob||!level_.inBoundsCell(mobPos))return false;mob->pos=mobPos;mob->HP=mobHp;mob->HT=mobHt;mob->setState(static_cast<Mob::State>(state));if(!dynamic_cast<SpecialMob*>(mob.get()))mob->restorePersistentState(mobSpecial);if(DungeonNPC*npc=dynamic_cast<DungeonNPC*>(mob.get()))npc->restoreCorpse(corpse!=0);if(Goo*goo=dynamic_cast<Goo*>(mob.get()))goo->restorePumpedUp((bossFlags&1u)!=0);if(ZoneBoss*boss=dynamic_cast<ZoneBoss*>(mob.get()))boss->restoreBossState((bossFlags&2u)!=0,(bossFlags&4u)!=0,version>=12u?bossTimer:5);if(Mimic*mimic=dynamic_cast<Mimic*>(mob.get()))for(auto&item:loot)mimic->addItem(std::move(item));if(SpecialMob*special=dynamic_cast<SpecialMob*>(mob.get())){if(loot.size()>1)return false;if(!loot.empty()){Weapon*weapon=dynamic_cast<Weapon*>(loot.front().get());if(!weapon)return false;loot.front().release();special->restoreWeapon(std::unique_ptr<Weapon>(weapon));}}if(Thief*thief=dynamic_cast<Thief*>(mob.get())){if(loot.size()>1)return false;if(!loot.empty())thief->restoreStolen(std::move(loot.front()));}if(!restoreBuffs(*mob,savedMobBuffs))return false;level_.addMob(std::move(mob));
}
    uint32_t heapCount=0;if(!r.get(heapCount)||heapCount>256)return false;for(uint32_t i=0;i<heapCount;++i){int32_t heapPos=0,type=0;uint32_t count=0;if(!r.get(heapPos)||!level_.inBoundsCell(heapPos)||!r.get(type)||type<static_cast<int>(Heap::Type::Heap)||type>static_cast<int>(Heap::Type::Hidden)||!r.get(count)||count>64)return false;for(uint32_t j=0;j<count;++j){std::unique_ptr<Item> item=readItem(r,version);if(!item)return false;Heap* heap=level_.drop(std::move(item),heapPos);if(!heap)return false;heap->type=static_cast<Heap::Type>(type);}}
    hero_.belongings().reset(&hero_);hero_.belongings().backpack().setSize(Belongings::BackpackSize+std::max<int32_t>(0,enduranceUpgrades)*3);uint32_t backpackCount=0;if(!r.get(backpackCount)||backpackCount>128)return false;for(uint32_t i=0;i<backpackCount;++i){std::unique_ptr<Item> item=readItem(r,version);if(!item||!hero_.belongings().backpack().collect(std::move(item)))return false;}
    for(int slot=0;slot<4;++slot){uint8_t present=0;if(!r.get(present))return false;if(!present)continue;std::unique_ptr<Item> item=readItem(r,version);if(!item)return false;if(slot==0){KindOfWeapon* raw=dynamic_cast<KindOfWeapon*>(item.get());if(!raw)return false;item.release();hero_.belongings().equipWeapon(std::unique_ptr<KindOfWeapon>(raw));}else if(slot==1){Armor* raw=dynamic_cast<Armor*>(item.get());if(!raw)return false;item.release();hero_.belongings().equipArmor(std::unique_ptr<Armor>(raw));}else{Ring* raw=dynamic_cast<Ring*>(item.get());if(!raw)return false;item.release();if(!hero_.belongings().equipRing(std::unique_ptr<Ring>(raw)))return false;}}
    int32_t quickSlot=0,quickIndex=-1;if(!r.get(quickSlot)||quickSlot<0||quickSlot>static_cast<int>(QuickSlotKind::Item)||(version>=18u&&!r.get(quickIndex)))return false;const std::vector<Item*>restoredItems=hero_.belongings().allItems();if(quickIndex>=0&&quickIndex<static_cast<int32_t>(restoredItems.size()))hero_.primaryQuickSlot().setItem(static_cast<QuickSlotKind>(quickSlot),restoredItems[static_cast<std::size_t>(quickIndex)]);else hero_.primaryQuickSlot().setClass(static_cast<QuickSlotKind>(quickSlot));
    if(!r.done()||!level_.inBoundsCell(heroPos)||ht<1||hp<1||hp>ht) return false;
    hero_.pos=heroPos;hero_.HP=hp;hero_.HT=ht;hero_.restoreProgress(strength,heroLevel,heroExp);hero_.restoreUpgrades(hpUpgrades,strengthUpgrades,enduranceUpgrades,luckUpgrades);hero_.setSubClass(static_cast<HeroSubClass>(heroSubClass));hero_.bindLevel(&level_);hunger_.restoreLevel(hungerLevel);hunger_.bind(&hero_);hero_.bindHunger(&hunger_);
    if(!restoreBuffs(hero_,savedHeroBuffs))return false;
scheduler_.addDelayed(&hero_,-std::numeric_limits<float>::min());scheduler_.add(&hunger_);for(Buff*buff:hero_.activeBuffs())if(buff)scheduler_.add(buff);for(auto& mob:level_.mobsMutable()){mob->bind(this);if(mob->isAlive())scheduler_.add(mob.get());for(Buff*buff:mob->activeBuffs())if(buff)scheduler_.add(buff);}if(WandOfMagicMissile* wand=hero_.belongings().getItem<WandOfMagicMissile>())wand->activate(hero_,scheduler_);scheduler_.process(&hero_);started_=true;observe();return true;
}

} // namespace pdv
