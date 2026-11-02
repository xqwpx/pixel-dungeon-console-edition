#pragma once

#include <string>

namespace pdv {
namespace Localization {

enum class Language : unsigned char { English, Russian, Portuguese, Spanish };

void setLanguage(Language language);
Language language();

std::string itemName(const std::string& english);
const char* action(const char* english);
std::string text(const char* english);
std::string text(const std::string& english);

} // namespace Localization
} // namespace pdv
