#pragma once

#include <sstream>
#include <string>

namespace pdv {

// libstdc++ shipped with the PS3 toolchain does not provide std::to_string.
// Keep numeric UI text portable without making gameplay platform-specific.
template <typename T>
std::string numberToString(const T& value) {
    std::ostringstream stream;
    stream << value;
    return stream.str();
}

} // namespace pdv
