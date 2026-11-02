#pragma once
#include <memory>
#include <cstdint>
#include <string>
#include "game/items/Item.h"

namespace pdv {

enum class ScrollKind {
    Identify,
    Teleportation,
    RemoveCurse,
    Recharging,
    MagicMapping,
    Challenge,
    Terror,
    Lullaby,
    PsionicBlast,
    MirrorImage,
    Upgrade,
    Enchantment,
    WipeOut
};

class Scroll : public Item {
public:
    static constexpr float TimeToRead = 1.0f;
    static constexpr int Count = 12;

    explicit Scroll(ScrollKind kind);

    static void initLabels();

    ScrollKind kind() const { return kind_; }
    bool sameKind(const Item&other)const override{const Scroll*value=dynamic_cast<const Scroll*>(&other);return value&&value->kind_==kind_;}
    std::string rune() const;

    bool isKnown() const;
    void setKnown();
    Item& identify() override;
    int price() const override;

    bool isIdentified() const override { return isKnown(); }
    bool isUpgradable() const override { return false; }

    int image() const override;
    std::string displayName() const override;
    std::string description() const;

    bool requiresInventoryTarget() const;
    std::vector<ItemAction> actions(const Hero&) const override { return {ItemAction::Read,ItemAction::Drop}; }
    const char* selectionTitle() const;

    std::unique_ptr<Item> clone() const override;

    static const char* kindName(ScrollKind kind);
    static int statusIndex(ScrollKind kind);
    static uint16_t knownMask();
    static void restoreKnownMask(uint16_t mask);

private:
    ScrollKind kind_;
};

class ScrollOfUpgrade : public Scroll {
public:
    ScrollOfUpgrade() : Scroll(ScrollKind::Upgrade) {}
    std::unique_ptr<Item> clone() const override {
        return std::unique_ptr<Item>(new ScrollOfUpgrade(*this));
    }
};

class ScrollOfEnchantment : public Scroll {
public:
    ScrollOfEnchantment() : Scroll(ScrollKind::Enchantment) {}
    std::unique_ptr<Item> clone() const override {
        return std::unique_ptr<Item>(new ScrollOfEnchantment(*this));
    }
};

} // namespace pdv
