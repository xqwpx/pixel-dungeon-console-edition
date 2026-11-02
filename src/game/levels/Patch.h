#pragma once
#include <array>
#include "game/levels/Level.h"
namespace pdv {
class Patch{
public: static const std::array<bool,Level::Length>& generate(float seed,int nGen);
private: static std::array<bool,Level::Length> cur_,off_;
};}
