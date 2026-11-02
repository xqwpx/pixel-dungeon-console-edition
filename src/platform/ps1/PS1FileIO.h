#pragma once

#include <cstddef>
#include <cstring>
#include <string>
#include <vector>

#include "platform/ps1/PS1MemoryCard.h"

// Small stdio facade for the existing save serializer. Every logical file is
// stored through PS1MemoryCard, so its BIOS card header and animated icon are
// retained without duplicating the serializer for the console build.
struct FILE {
    std::string name;
    std::vector<unsigned char> bytes;
    std::size_t position = 0;
    bool writing = false;
};

constexpr int SEEK_SET = 0;
constexpr int SEEK_CUR = 1;
constexpr int SEEK_END = 2;

namespace std {
inline FILE* fopen(const char* name, const char* mode) {
    if (!name || !mode) return nullptr;
    FILE* file = new FILE();
    file->name = name;
    file->writing = mode[0] == 'w';
    if (!file->writing && !pdv::PS1MemoryCard::readFile(file->name, file->bytes)) {
        delete file;
        return nullptr;
    }
    return file;
}
inline int fclose(FILE* file) {
    if (!file) return -1;
    const bool ok = !file->writing || pdv::PS1MemoryCard::writeFile(
        file->name, file->bytes.data(), file->bytes.size());
    delete file;
    return ok ? 0 : -1;
}
inline size_t fread(void* out, size_t size, size_t count, FILE* file) {
    if (!file || !out || size == 0) return 0;
    const size_t available = file->position < file->bytes.size() ? file->bytes.size() - file->position : 0;
    const size_t amount = count < available / size ? count * size : available - available % size;
    if (amount) memcpy(out, file->bytes.data() + file->position, amount);
    file->position += amount;
    return amount / size;
}
inline size_t fwrite(const void* data, size_t size, size_t count, FILE* file) {
    if (!file || !data || !file->writing || size == 0) return 0;
    const size_t amount = size * count;
    if (file->position + amount > file->bytes.size()) file->bytes.resize(file->position + amount);
    memcpy(file->bytes.data() + file->position, data, amount);
    file->position += amount;
    return count;
}
inline int fseek(FILE* file, long offset, int origin) {
    if (!file) return -1;
    long base = origin == SEEK_END ? static_cast<long>(file->bytes.size()) : origin == SEEK_CUR ? static_cast<long>(file->position) : 0;
    const long destination = base + offset;
    if (destination < 0) return -1;
    file->position = static_cast<size_t>(destination);
    return 0;
}
inline long ftell(FILE* file) { return file ? static_cast<long>(file->position) : -1; }
inline void rewind(FILE* file) { if (file) file->position = 0; }
inline int fflush(FILE*) { return 0; }
inline int remove(const char* name) { return pdv::PS1MemoryCard::removeFile(name ? name : "") ? 0 : -1; }
inline int rename(const char* from, const char* to) {
    if (!from || !to) return -1;
    vector<unsigned char> bytes;
    if (!pdv::PS1MemoryCard::readFile(from, bytes) ||
        !pdv::PS1MemoryCard::writeFile(to, bytes.data(), bytes.size())) return -1;
    return pdv::PS1MemoryCard::removeFile(from) ? 0 : -1;
}
} // namespace std
