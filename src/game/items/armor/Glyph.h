#pragma once
#include <memory>
#include <string>
namespace pdv {
class Armor; class Char;
enum class GlyphKind { Bounce=1, Affection, AntiEntropy, Multiplicity, Potential, Metabolism, Stench, Viscosity, Displacement, Entanglement, AutoRepair };
class Glyph {
public:
    virtual ~Glyph() = default;
    virtual GlyphKind kind() const = 0;
    virtual int proc(Armor&, Char&, Char&, int damage) = 0;
    virtual std::string name(const std::string&) const = 0;
    virtual std::unique_ptr<Glyph> clone() const = 0;
    static std::unique_ptr<Glyph> create(GlyphKind kind);
    static std::unique_ptr<Glyph> random();
};
} // namespace pdv
