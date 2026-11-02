#pragma once
namespace pdv { class ITimeSource { public: virtual ~ITimeSource() = default; virtual double seconds() const = 0; }; }
