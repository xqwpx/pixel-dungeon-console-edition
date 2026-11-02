#pragma once

#include <string>
#include <vector>
#include "game/items/Item.h"

namespace pdv {

class Bag;
class Hero;
class Item;
class Scroll;

enum class InventoryMode {
    All,
    Unidentified,
    Upgradable,
    Enchantable
};

struct InventorySlot {
    Item* item = nullptr;
    int col = 0;
    int row = 0;
    bool enabled = false;
    bool equipped = false;
};

class InventoryModel {
public:
    static constexpr int Columns = 6;
    // 32 px keeps multi-digit stack counts clear while retaining six rows.
    static constexpr int SlotSize = 32;
    static constexpr int SlotMargin = 1;
    static constexpr int TitleHeight = 12;

    void bind(Hero* hero);

    void open(
        InventoryMode mode = InventoryMode::All,
        const std::string& title = "Backpack"
    );

    void close();

    bool openState() const { return open_; }

    void setMode(
        InventoryMode mode,
        const std::string& title
    );

    InventoryMode mode() const { return mode_; }

    const std::string& title() const { return title_; }

    const std::vector<InventorySlot>& slots() const {
        return slots_;
    }

    Item* selectedItem() const;
    int selectedIndex() const { return selected_; }

    void selectIndex(int index);
    void moveSelection(int dx, int dy);

    int tabCount() const {
        return static_cast<int>(tabs_.size());
    }

    int activeTab() const { return activeTab_; }

    const std::string& tabName(int index) const;

    void nextTab();
    void previousTab();

    void rebuild();

    bool accepts(const Item& item) const;
    bool openContext();
    void closeContext();
    bool contextOpen() const { return contextOpen_; }
    const std::vector<ItemAction>& contextActions() const { return contextActions_; }
    int contextSelection() const { return contextSelected_; }
    ItemAction selectedAction() const;
    void moveContextSelection(int delta);

private:
    struct Tab {
        Bag* bag = nullptr;
        std::string name;
    };

    int findNearest(
        int col,
        int row,
        int dx,
        int dy
    ) const;

    Hero* hero_ = nullptr;

    bool open_ = false;
    InventoryMode mode_ = InventoryMode::All;

    std::string title_ = "Backpack";

    std::vector<Tab> tabs_;
    int activeTab_ = 0;

    std::vector<InventorySlot> slots_;
    int selected_ = -1;
    bool contextOpen_ = false;
    std::vector<ItemAction> contextActions_;
    int contextSelected_ = -1;
};

} // namespace pdv
