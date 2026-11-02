#include "game/items/armor/Armor.h"
#include "game/utils/Random.h"
#include "game/actors/Hero.h"
#include <sstream>

namespace pdv {

Armor::Armor(int tier)
    : tier_(tier) {

    strength_ = typicalSTR();
}
Armor::Armor(const Armor& other): Item(other), tier_(other.tier_), strength_(other.strength_) { if(other.glyph_) glyph_=other.glyph_->clone(); }
Armor& Armor::operator=(const Armor& other){ if(this!=&other){Item::operator=(other);tier_=other.tier_;strength_=other.strength_;glyph_=other.glyph_?other.glyph_->clone():nullptr;}return *this; }

int Armor::DR() const {
    return tier_ * (
        2 +
        level_ +
        (glyph_ ? 1 : 0)
    );
}

Item& Armor::upgrade(int n) {
    for (int i = 0; i < n; ++i) {
        strength_--;
        Item::upgrade(1);
    }

    return *this;
}

Item& Armor::degrade(int n) {
    for (int i = 0; i < n; ++i) {
        strength_++;
        Item::degrade(1);
    }

    return *this;
}

void Armor::inscribe() {
    const int old=glyphId(); do { glyph_=Glyph::random(); } while(glyphId()==old);
}
void Armor::inscribe(GlyphKind kind){ glyph_=Glyph::create(kind); }
void Armor::inscribe(std::unique_ptr<Glyph> glyph){ glyph_=std::move(glyph); }
std::string Armor::displayName() const { return glyph_ ? glyph_->name(name_) : Item::displayName(); }

int Armor::price() const {
    int value=10*(1<<(tier_-1));
    if (glyph_) value=value*3/2;
    return priceWithState(value);
}

int Armor::typicalSTR() const {
    return 7 + tier_ * 2;
}

int Armor::typicalDR() const {
    return tier_ * 2;
}

int Armor::proc(Char& attacker, Char& defender, int damage) {
    if (glyph_) damage=glyph_->proc(*this,attacker,defender,damage);
    use();
    return damage;
}
std::vector<ItemAction> Armor::actions(const Hero& hero) const { return {hero.belongings().armor()==this?ItemAction::Unequip:ItemAction::Equip,ItemAction::Drop}; }
std::string Armor::info() const { std::ostringstream out; out<<Item::info()<<" | DR "<<DR()<<" | STR "<<STR(); return out.str(); }

} // namespace pdv
