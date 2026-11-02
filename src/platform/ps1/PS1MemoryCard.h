#pragma once

#include <cstddef>
#include <string>
#include <vector>

namespace pdv::PS1MemoryCard {

void initialize();
bool readFile(const std::string& name, std::vector<unsigned char>& data);
bool writeFile(const std::string& name, const void* data, std::size_t size);
bool removeFile(const std::string& name);

} // namespace pdv::PS1MemoryCard
