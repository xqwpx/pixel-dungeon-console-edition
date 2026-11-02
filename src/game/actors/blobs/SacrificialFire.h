#pragma once
#include "game/actors/blobs/Blob.h"
#include "game/actors/buffs/FlavourBuff.h"
namespace pdv { class Char;
class SacrificialFire:public Blob { public: BlobKind kind()const override{return BlobKind::SacrificialFire;} void seed(int cell,int amount) override; static void sacrifice(Char&); protected:void evolve() override; private:int pos_=-1; };
class Marked:public FlavourBuff { public: static constexpr float Duration=5.0f; };
}
