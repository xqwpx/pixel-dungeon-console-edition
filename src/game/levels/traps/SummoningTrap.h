#pragma once
namespace pdv { class Dungeon; class Char; class SummoningTrap { public: static constexpr float Delay=2.0f; static void trigger(Dungeon&,int,Char*); }; }
