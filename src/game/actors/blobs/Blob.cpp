#include "game/actors/blobs/Blob.h"

#include "game/Dungeon.h"
#include "game/levels/Level.h"

#include <algorithm>

namespace pdv {

Blob::Blob() {
    cur_.fill(0);
    off_.fill(0);
}

bool Blob::act() {
    spend(TICK);

    if (volume_ > 0) {
        volume_ = 0;
        evolve();
        cur_.swap(off_);
        off_.fill(0);
    }

    next();
    return true;
}

void Blob::seed(int cell, int amount) {
    if (cell < 0 ||
        cell >= LevelLength ||
        amount <= 0) {
        return;
    }

    cur_[static_cast<std::size_t>(cell)] += amount;
    volume_ += amount;
}

void Blob::clear(int cell) {
    if (cell < 0 ||
        cell >= LevelLength) {
        return;
    }

    volume_ -= cur_[static_cast<std::size_t>(cell)];
    cur_[static_cast<std::size_t>(cell)] = 0;

    if (volume_ < 0) {
        volume_ = 0;
    }
}

void Blob::setValueAt(int cell, int value) {
    if (cell < 0 ||
        cell >= LevelLength) {
        return;
    }

    const int old =
        cur_[static_cast<std::size_t>(cell)];

    cur_[static_cast<std::size_t>(cell)] =
        std::max(0, value);

    volume_ +=
        cur_[static_cast<std::size_t>(cell)] -
        old;

    if (volume_ < 0) {
        volume_ = 0;
    }
}

void Blob::evolve() {
    if (!dungeon_) {
        return;
    }

    const Level& level =
        dungeon_->level();

    const auto& solid =
        level.solidMap();

    for (int y = 1;
         y < Level::Height - 1;
         ++y) {

        int from =
            y * Level::Width + 1;

        const int to =
            from +
            Level::Width - 2;

        for (int pos = from;
             pos < to;
             ++pos) {

            if (!solid[
                    static_cast<std::size_t>(
                        pos
                    )
                ]) {

                int count = 1;
                int sum =
                    cur_[
                        static_cast<std::size_t>(
                            pos
                        )
                    ];

                const int neighbours[4] = {
                    pos - 1,
                    pos + 1,
                    pos - Level::Width,
                    pos + Level::Width
                };

                for (int n : neighbours) {
                    if (!solid[
                            static_cast<std::size_t>(
                                n
                            )
                        ]) {

                        sum +=
                            cur_[
                                static_cast<std::size_t>(
                                    n
                                )
                            ];

                        ++count;
                    }
                }

                const int value =
                    sum >= count
                    ? (sum / count) - 1
                    : 0;

                off_[
                    static_cast<std::size_t>(
                        pos
                    )
                ] = value;

                volume_ += value;

            } else {
                off_[
                    static_cast<std::size_t>(
                        pos
                    )
                ] = 0;
            }
        }
    }
}

} // namespace pdv
