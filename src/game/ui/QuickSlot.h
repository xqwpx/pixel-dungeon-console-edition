#pragma once
namespace pdv {
class Hero; class Item;
enum class QuickSlotKind { None, Dart, WandOfMagicMissile, Boomerang, Wand, Missile, Item };
class QuickSlot {
public:
    void clear(){ kind_=QuickSlotKind::None; instance_=nullptr; }
    void setClass(QuickSlotKind kind){ kind_=kind; instance_=nullptr; }
    void setItem(QuickSlotKind kind, Item* item){ kind_=kind; instance_=item; }
    QuickSlotKind kind() const { return kind_; }
    Item* select(const Hero& hero) const;
private:
    QuickSlotKind kind_=QuickSlotKind::None;
    Item* instance_=nullptr;
};
} // namespace pdv
