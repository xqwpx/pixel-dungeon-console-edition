#pragma once
namespace pdv { class ILogger { public: virtual ~ILogger() = default; virtual void info(const char*) = 0; virtual void error(const char*) = 0; }; }
