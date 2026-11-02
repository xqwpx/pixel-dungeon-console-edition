#pragma once

#include <cstdint>
#include <random>
#include <vector>

namespace pdv {

class Random {
public:
    static void seed(uint32_t seed);

    static float Float();
    static float Float(float max);
    static float Float(float min, float max);

    static int Int(int max);
    static int Int(int min, int max);
    static int IntRange(int min, int max);

    static int NormalIntRange(int min, int max);

    template<class T>
    static T* element(const std::vector<T*>& values) {
        if (values.empty()) {
            return nullptr;
        }

        return values[
            static_cast<std::size_t>(
                Int(static_cast<int>(values.size()))
            )
        ];
    }

private:
    static std::mt19937 rng_;
};

} // namespace pdv
