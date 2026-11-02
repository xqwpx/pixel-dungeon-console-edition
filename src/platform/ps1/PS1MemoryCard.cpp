#include "platform/ps1/PS1MemoryCard.h"

extern "C" {
#include <psxapi.h>
}

namespace pdv::PS1MemoryCard {
namespace {
constexpr int OpenReadOnly = 0;
constexpr int OpenWriteOnly = 1;
constexpr int OpenCreate = 0x0200;
constexpr int OpenTruncate = 0x0400;
constexpr std::size_t SaveHeaderSize = 512;

void appendIcon(std::vector<unsigned char>& block) {
    block.resize(SaveHeaderSize);
    for (std::size_t index = 0; index < SaveHeaderSize; ++index) block[index] = 0;
    block[0] = 'S'; block[1] = 'C'; block[2] = 0x13; block[3] = 1;
    const char title[] = "Pixel Dungeon";
    for (std::size_t index = 0; index < sizeof(title) - 1; ++index) block[4 + index] = title[index];
    const uint16_t palette[16] = {0x0000,0x0000,0x7fff,0x4210,0x001f,0x03e0,0x7c00,0x7fe0,0x7c1f,0x03ff,0x7fff,0x4631,0x294a,0x14a5,0x56b5,0x7fff};
    for (std::size_t index = 0; index < 16; ++index) { block[96 + index * 2] = palette[index] & 0xff; block[97 + index * 2] = palette[index] >> 8; }
    for (int frame = 0; frame < 3; ++frame) for (int y = 0; y < 16; ++y) for (int x = 0; x < 16; x += 2) {
        const int dx = x - 8, dy = y - 8;
        const bool body = dx * dx + dy * dy < 49;
        const int glow = body && ((x + y + frame) % 5 == 0);
        const unsigned char pixel0 = body ? static_cast<unsigned char>(glow ? 7 : 4) : 0;
        const unsigned char pixel1 = body ? static_cast<unsigned char>(glow ? 7 : 4) : 0;
        block[128 + frame * 128 + y * 8 + x / 2] = static_cast<unsigned char>((pixel1 << 4) | pixel0);
    }
}

std::string pathFor(const std::string& name) {
    return "bu00:PDCE_" + name;
}
}

void initialize() {
    InitCARD(1);
    StartCARD();
    _bu_init();
}

bool readFile(const std::string& name, std::vector<unsigned char>& data) {
    const std::string path = pathFor(name);
    const int fd = open(path.c_str(), OpenReadOnly);
    if (fd < 0) return false;
    const int end = lseek(fd, 0, SEEK_END);
    if (end <= 0 || end > 65536 || lseek(fd, 0, SEEK_SET) < 0) { close(fd); return false; }
    data.resize(static_cast<std::size_t>(end));
    const bool ok = read(fd, data.data(), static_cast<std::size_t>(end)) == end;
    close(fd);
    if (!ok || data.size() < SaveHeaderSize || data[0] != 'S' || data[1] != 'C') { data.clear(); return false; }
    const std::size_t payload = data.size() - SaveHeaderSize;
    for (std::size_t index = 0; index < payload; ++index) data[index] = data[SaveHeaderSize + index];
    data.resize(payload);
    return true;
}

bool writeFile(const std::string& name, const void* data, std::size_t size) {
    const std::string path = pathFor(name);
    std::vector<unsigned char> file;
    appendIcon(file);
    const unsigned char* source = static_cast<const unsigned char*>(data);
    for (std::size_t index = 0; index < size; ++index) file.push_back(source[index]);
    const int fd = open(path.c_str(), OpenWriteOnly | OpenCreate | OpenTruncate);
    if (fd < 0) return false;
    const bool ok = write(fd, file.data(), file.size()) == static_cast<int>(file.size());
    close(fd);
    return ok;
}

bool removeFile(const std::string& name) {
    return erase(pathFor(name).c_str()) == 0;
}

} // namespace pdv::PS1MemoryCard
