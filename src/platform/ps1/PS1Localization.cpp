#include "core/Localization.h"

namespace pdv::Localization {
namespace { Language current = Language::English; }
void setLanguage(Language value) { current = value; }
Language language() { return current; }
std::string itemName(const std::string& english) { return english; }
const char* action(const char* english) { return english; }
std::string text(const char* english) { return std::string(english ? english : ""); }
std::string text(const std::string& english) { return english; }
} // namespace pdv::Localization
