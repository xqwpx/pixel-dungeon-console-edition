#include "game/items/ItemStatusHandler.h"

#include "game/utils/Random.h"

#include <vector>

namespace pdv {

void ItemStatusHandler::init(
    const std::array<std::string, Capacity>& labels,
    const std::array<int, Capacity>& images
) {
    std::vector<std::string> labelsLeft(labels.begin(), labels.end());
    std::vector<int> imagesLeft(images.begin(), images.end());

    known_.fill(false);

    for (int i = 0; i < Capacity; ++i) {
        const int index = Random::Int(static_cast<int>(labelsLeft.size()));

        labels_[static_cast<std::size_t>(i)] =
            labelsLeft[static_cast<std::size_t>(index)];

        images_[static_cast<std::size_t>(i)] =
            imagesLeft[static_cast<std::size_t>(index)];

        labelsLeft.erase(labelsLeft.begin() + index);
        imagesLeft.erase(imagesLeft.begin() + index);
    }
}

const std::string& ItemStatusHandler::label(int itemIndex) const {
    static const std::string empty;
    if (itemIndex < 0 || itemIndex >= Capacity) return empty;
    return labels_[static_cast<std::size_t>(itemIndex)];
}

int ItemStatusHandler::image(int itemIndex) const {
    if (itemIndex < 0 || itemIndex >= Capacity) return 0;
    return images_[static_cast<std::size_t>(itemIndex)];
}

bool ItemStatusHandler::isKnown(int itemIndex) const {
    return itemIndex >= 0 &&
           itemIndex < Capacity &&
           known_[static_cast<std::size_t>(itemIndex)];
}

void ItemStatusHandler::know(int itemIndex) {
    if (itemIndex < 0 || itemIndex >= Capacity) return;

    known_[static_cast<std::size_t>(itemIndex)] = true;

    if (knownCount() == Capacity - 1) {
        for (int i = 0; i < Capacity; ++i) {
            if (!known_[static_cast<std::size_t>(i)]) {
                known_[static_cast<std::size_t>(i)] = true;
                break;
            }
        }
    }
}

int ItemStatusHandler::knownCount() const {
    int result = 0;
    for (bool value : known_) if (value) ++result;
    return result;
}

bool ItemStatusHandler::allKnown() const {
    return knownCount() == Capacity;
}

uint16_t ItemStatusHandler::knownMask()const{uint16_t mask=0;for(int i=0;i<Capacity;++i)if(known_[static_cast<std::size_t>(i)])mask|=static_cast<uint16_t>(1u<<i);return mask;}
void ItemStatusHandler::restoreKnownMask(uint16_t mask){for(int i=0;i<Capacity;++i)known_[static_cast<std::size_t>(i)]=(mask&(1u<<i))!=0;}

} // namespace pdv
