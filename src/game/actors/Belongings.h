#pragma once
#include <memory>
#include <vector>
#include "game/items/Bag.h"

namespace pdv {
class Armor; class Hero; class KindOfWeapon; class Ring;
class Belongings {
public:
    static constexpr int BackpackSize=19;
    explicit Belongings(Hero* owner=nullptr); ~Belongings();
    Belongings(const Belongings&)=delete; Belongings& operator=(const Belongings&)=delete;
    void reset(Hero* owner);
    Hero* owner() const { return owner_; }
    Bag& backpack(){ return backpack_; } const Bag& backpack() const { return backpack_; }
    KindOfWeapon* weapon() const { return weapon_.get(); }
    Armor* armor() const { return armor_.get(); }
    Ring* ring1() const { return ring1_.get(); }
    Ring* ring2() const { return ring2_.get(); }
    void equipWeapon(std::unique_ptr<KindOfWeapon> weapon);
    void equipArmor(std::unique_ptr<Armor> armor);
    bool equipRing(std::unique_ptr<Ring> ring);
    std::unique_ptr<KindOfWeapon> takeWeapon();
    std::unique_ptr<Armor> takeArmor();
    std::unique_ptr<Ring> takeRing(Ring* ring);

    std::vector<Item*> allItems() const;
    int charge(bool full);
    int uncurseEquipped();
    int uncurseAll();
    void observeEquipped();
    void resurrect(int depth);

    template<class T> T* getItem() const { return backpack_.template findFirst<T>(); }
    // Keys are reusable on any floor; only their type determines the lock
    // they can open.
    template<class T> T* getKey() const { return backpack_.template findKey<T>(); }
    std::unique_ptr<Item> detach(Item* item){ return backpack_.detach(item); }

private:
    Hero* owner_=nullptr;
    Bag backpack_{BackpackSize, "backpack"};
    std::unique_ptr<KindOfWeapon> weapon_;
    std::unique_ptr<Armor> armor_;
    std::unique_ptr<Ring> ring1_;
    std::unique_ptr<Ring> ring2_;
};
} // namespace pdv
