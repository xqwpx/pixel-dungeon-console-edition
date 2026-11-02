#pragma once
#include <string>
namespace pdv { enum class FileRoot { Assets, UserData, Cache, Temporary }; class IFileSystem { public: virtual ~IFileSystem() = default; virtual std::string path(FileRoot root, const std::string& relative) const = 0; }; }
