#include <cstddef>
#include <stdlib.h>

// PSn00bSDK deliberately does not ship the normal libstdc++ runtime.  The
// game's ownership code needs the replaceable allocation operators, backed by
// the SDK's C allocator.  Exceptions remain disabled for the PS1 target.
void* operator new(std::size_t size) {
    if (void* memory = malloc(size)) return memory;
    __builtin_trap();
}

void* operator new[](std::size_t size) {
    if (void* memory = malloc(size)) return memory;
    __builtin_trap();
}

void operator delete(void* memory) noexcept { free(memory); }
void operator delete[](void* memory) noexcept { free(memory); }
void operator delete(void* memory, std::size_t) noexcept { free(memory); }
void operator delete[](void* memory, std::size_t) noexcept { free(memory); }

// Static C++ objects in the shared game model register destructors through
// atexit. A PS1 executable never returns from main, so retaining callbacks
// would only consume RAM; accepting the registration is sufficient.
extern "C" int atexit(void (*)()) { return 0; }

// libpsn00b's freestanding runtime does not include libm. The dungeon only
// needs deterministic, low-precision values for layout and field-of-view.
extern "C" float sqrtf(float value) {
    if (value <= 0.0f) return 0.0f;
    float estimate = value > 1.0f ? value : 1.0f;
    for (int iteration = 0; iteration < 8; ++iteration)
        estimate = (estimate + value / estimate) * 0.5f;
    return estimate;
}
extern "C" float floorf(float value) {
    int whole = static_cast<int>(value);
    if (value < 0.0f && static_cast<float>(whole) != value) --whole;
    return static_cast<float>(whole);
}
extern "C" double floor(double value) {
    int whole = static_cast<int>(value);
    if (value < 0.0 && static_cast<double>(whole) != value) --whole;
    return static_cast<double>(whole);
}
extern "C" double cos(double value) {
    // Range reduction plus a small even polynomial is ample for shadow arcs.
    constexpr double Pi = 3.14159265358979323846;
    while (value > Pi) value -= Pi * 2.0;
    while (value < -Pi) value += Pi * 2.0;
    const double square = value * value;
    return 1.0 - square * (0.5 - square * (1.0 / 24.0 - square / 720.0));
}
namespace {
double ps1Log(double value) {
    if (value <= 0.0) return -80.0;
    int exponent = 0;
    while (value > 2.0) { value *= 0.5; ++exponent; }
    while (value < 1.0) { value *= 2.0; --exponent; }
    const double ratio = (value - 1.0) / (value + 1.0);
    const double square = ratio * ratio;
    double term = ratio, sum = 0.0;
    for (int index = 1; index < 12; index += 2) { sum += term / index; term *= square; }
    return sum * 2.0 + exponent * 0.6931471805599453;
}
double ps1Exp(double value) {
    if (value < -40.0) return 0.0;
    if (value > 40.0) value = 40.0;
    int halves = 0;
    while (value > 0.5) { value *= 0.5; ++halves; }
    while (value < -0.5) { value *= 0.5; ++halves; }
    double sum = 1.0, term = 1.0;
    for (int index = 1; index < 14; ++index) { term *= value / index; sum += term; }
    while (halves-- > 0) sum *= sum;
    return sum;
}
}
extern "C" double pow(double base, double exponent) { return ps1Exp(exponent * ps1Log(base)); }
extern "C" float powf(float base, float exponent) { return static_cast<float>(pow(base, exponent)); }
extern "C" double asin(double value) {
    if (value > 1.0) value = 1.0;
    if (value < -1.0) value = -1.0;
    const double square = value * value;
    return value * (1.0 + square * (1.0 / 6.0 + square * (3.0 / 40.0 + square * 5.0 / 112.0)));
}
