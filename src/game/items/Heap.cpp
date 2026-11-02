#include "game/items/Heap.h"
#include "game/items/Item.h"
#include "game/items/scrolls/Scroll.h"
#include "game/items/ConsumableCatalog.h"
#include "game/items/SpecialItems.h"

namespace pdv {

Heap::Heap(int cell) : pos(cell) {}

Item* Heap::peek() const {
    return items_.empty() ? nullptr : items_.front().get();
}

std::unique_ptr<Item> Heap::pickUp() {
    if (items_.empty()) return nullptr;

    auto result = std::move(items_.front());
    items_.pop_front();
    return result;
}

void Heap::drop(std::unique_ptr<Item> item) {
    if (!item) return;

    if (item->stackable()) {
        for (auto& existing : items_) {
            if (existing && existing->sameKind(*item)) {
                existing->quantity(existing->quantity() + item->quantity());
                return;
            }
        }
    }

    items_.push_front(std::move(item));
}

void Heap::burn(){
    if(type!=Type::Heap)return;
    for(auto it=items_.begin();it!=items_.end();){
        Item* item=it->get();
        if(dynamic_cast<Scroll*>(item)||(dynamic_cast<SpecialItem*>(item)&&static_cast<SpecialItem*>(item)->kind()==SpecialKind::Dewdrop)){it=items_.erase(it);continue;}
        if(MysteryMeat* meat=dynamic_cast<MysteryMeat*>(item)){const int count=meat->quantity();std::unique_ptr<PreparedFood> cooked(new PreparedFood(PreparedFoodKind::ChargrilledMeat));cooked->quantity(count);*it=std::move(cooked);}
        ++it;
    }
}

void Heap::freeze(){
    if(type!=Type::Heap)return;
    for(auto& item:items_)if(MysteryMeat* meat=dynamic_cast<MysteryMeat*>(item.get())){const int count=meat->quantity();std::unique_ptr<PreparedFood> frozen(new PreparedFood(PreparedFoodKind::FrozenCarpaccio));frozen->quantity(count);item=std::move(frozen);}
}
void Heap::replaceTop(std::unique_ptr<Item> item){if(!item)return;if(items_.empty())items_.push_front(std::move(item));else items_.front()=std::move(item);}

} // namespace pdv
