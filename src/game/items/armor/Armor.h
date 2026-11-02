#pragma once
#include "game/items/Item.h"
#include "game/items/armor/Glyph.h"
#include <memory>

namespace pdv {

class Char;

class Armor : public Item {
public:
    explicit Armor(int tier);
    Armor(const Armor& other);
    Armor& operator=(const Armor& other);

    int tier() const { return tier_; }
    int STR() const { return strength_; }

    int DR() const;

    Item& upgrade(int n = 1) override;
    Item& degrade(int n = 1) override;

    void inscribe();
    void inscribe(GlyphKind kind);
    void inscribe(std::unique_ptr<Glyph> glyph);
    bool isInscribed() const { return glyph_ != nullptr; }
    int glyphId() const { return glyph_ ? static_cast<int>(glyph_->kind()) : 0; }
    Glyph* glyph() const { return glyph_.get(); }
    std::string displayName() const override;
    int price() const override;
    int typicalSTR() const;
    int typicalDR() const;

    virtual int proc(Char& attacker, Char& defender, int damage);
    int maxDurability(int level) const override { return 6 * (level < 16 ? 16 - level : 1); }
    std::vector<ItemAction> actions(const Hero& hero) const override;
    std::string info() const override;

private:
    int tier_ = 1;
    int strength_ = 9;
    std::unique_ptr<Glyph> glyph_;
};

} // namespace pdv
