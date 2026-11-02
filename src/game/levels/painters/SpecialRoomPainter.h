#pragma once
#include "game/levels/Room.h"
namespace pdv { class Level; class SpecialRoomPainter { public: static void paint(Level&,Room&); static void garden(Level&,Room&); static void library(Level&,Room&); static void laboratory(Level&,Room&); }; }
