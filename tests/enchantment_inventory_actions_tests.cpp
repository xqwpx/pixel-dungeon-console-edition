#include <cassert>
#include <iostream>
#include <set>
#include "game/Dungeon.h"
#include "game/actors/buffs/Charm.h"
#include "game/actors/mobs/Rat.h"
#include "game/items/armor/ClothArmor.h"
#include "game/items/food/Food.h"
#include "game/items/Heap.h"
#include "game/items/potions/PotionOfHealing.h"
#include "game/items/scrolls/Scroll.h"
#include "game/items/wands/WandOfMagicMissile.h"
#include "game/items/weapon/melee/ShortSword.h"
#include "game/ui/InventoryModel.h"
#include "game/utils/Random.h"

using namespace pdv;
static bool has(const std::vector<ItemAction>& a,ItemAction v){for(ItemAction x:a)if(x==v)return true;return false;}

int main(){
    Random::seed(0x1700);
    Dungeon d; assert(d.startNew(HeroClass::Warrior,0x1700)); Hero& hero=d.hero();

    ShortSword enchanted; enchanted.enchant(EnchantmentKind::Fire);
    Rat rat; rat.HT=rat.HP=200;
    const int before=rat.HP;
    for(int i=0;i<20 && rat.HP==before;++i) enchanted.proc(hero,rat,8);
    assert(rat.HP<before);
    std::unique_ptr<Item> copiedItem=enchanted.clone();
    Weapon* copied=dynamic_cast<Weapon*>(copiedItem.get());
    assert(copied && copied->enchantment() && copied->enchantment()!=enchanted.enchantment());
    assert(copied->enchantmentId()==enchanted.enchantmentId());

    std::set<int> rolled;
    for(int i=0;i<300;++i){ShortSword w;w.enchant();rolled.insert(w.enchantmentId());}
    assert(rolled.count(static_cast<int>(EnchantmentKind::Fire)));
    assert(Enchantment::weight(EnchantmentKind::Fire)==10);
    assert(Enchantment::weight(EnchantmentKind::Death)==1);

    ClothArmor glyphArmor; glyphArmor.inscribe(GlyphKind::Affection);
    Rat attacker; attacker.pos=hero.pos+1;
    for(int i=0;i<30 && !attacker.hasBuff(typeid(Charm));++i) glyphArmor.proc(attacker,hero,5);
    assert(attacker.hasBuff(typeid(Charm)));
    std::unique_ptr<Item> armorCopyItem=glyphArmor.clone();
    Armor* armorCopy=dynamic_cast<Armor*>(armorCopyItem.get());
    assert(armorCopy && armorCopy->glyph() && armorCopy->glyph()!=glyphArmor.glyph());
    rolled.clear(); for(int i=0;i<100;++i){ClothArmor a;a.inscribe();rolled.insert(a.glyphId());}
    assert(rolled.size()>7);

    std::unique_ptr<ShortSword> spare(new ShortSword()); ShortSword* spareRaw=spare.get();
    assert(hero.belongings().backpack().collect(std::move(spare)));
    assert(d.equipItem(spareRaw)); assert(hero.belongings().weapon()==spareRaw);
    assert(d.unequipItem(spareRaw)); assert(hero.belongings().weapon()==nullptr);

    std::unique_ptr<ClothArmor> spareArmor(new ClothArmor()); ClothArmor* spareArmorRaw=spareArmor.get();
    assert(hero.belongings().backpack().collect(std::move(spareArmor)));
    assert(d.equipItem(spareArmorRaw)); assert(hero.belongings().armor()==spareArmorRaw);
    assert(d.unequipItem(spareArmorRaw)); assert(hero.belongings().armor()==nullptr);

    std::unique_ptr<PotionOfHealing> foodStack(new PotionOfHealing()); foodStack->quantity(3); PotionOfHealing* stackRaw=foodStack.get();
    assert(hero.belongings().backpack().collect(std::move(foodStack)));
    assert(d.dropItem(stackRaw)); assert(d.level().heapAt(hero.pos));
    assert(d.level().heapAt(hero.pos)->peek()->quantity()==3);

    ShortSword weapon; PotionOfHealing potion; Scroll scroll(ScrollKind::MagicMapping); Food food; WandOfMagicMissile wand;
    assert(has(weapon.actions(hero),ItemAction::Equip)&&has(weapon.actions(hero),ItemAction::QuickSlot));
    assert(has(potion.actions(hero),ItemAction::Drink)&&has(potion.actions(hero),ItemAction::Throw));
    assert(has(scroll.actions(hero),ItemAction::Read)&&!has(scroll.actions(hero),ItemAction::Throw));
    assert(has(food.actions(hero),ItemAction::Eat));
    assert(has(wand.actions(hero),ItemAction::Zap)&&has(wand.actions(hero),ItemAction::QuickSlot));

    InventoryModel model; model.bind(&hero); model.open(); assert(model.selectedItem());
    assert(model.openContext()); assert(model.contextOpen()); int first=model.contextSelection();
    model.moveContextSelection(1); assert(model.contextSelection()!=first || model.contextActions().size()==1);
    model.closeContext(); assert(!model.contextOpen()); assert(model.openState());
    model.close(); assert(!model.openState());

    std::cout<<"PIXEL DUNGEON CONSOLE EDITION v0.17 enchantment + inventory actions tests: OK\n";
}
