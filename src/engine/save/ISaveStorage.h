#pragma once
#include <cstdint>
#include <string>
#include <vector>
namespace pdv { class ISaveStorage { public: virtual ~ISaveStorage() = default; virtual bool read(const std::string&, std::vector<uint8_t>&) = 0; virtual bool writeAtomic(const std::string&, const std::vector<uint8_t>&) = 0; }; }
