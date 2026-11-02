#pragma once
#include <array>
#include <memory>
#include <vector>
#include "game/items/Item.h"
namespace pdv {
class Generator {
public:
    enum class Category { Weapon, Armor, Potion, Scroll, Wand, Ring, Seed, Food, Gold, Misc, Count };
    static void reset();
    static Category randomCategory();
    static std::unique_ptr<Item> random(int depth);
    static std::unique_ptr<Item> random(Category category, int depth);
    // Non-random catalogue used by the developer test arena.
    static std::vector<std::unique_ptr<Item>> allImplementedItems(int depth);
    static float baseProbability(Category category);
    static float currentProbability(Category category);
    static bool categoryImplemented(Category category);
    static const std::array<float,12>& potionProbabilities();
    static const std::array<float,12>& scrollProbabilities();
private:
    static std::array<float, static_cast<std::size_t>(Category::Count)> categoryProbs_;
};
}
