#pragma once
#include "game/actors/blobs/Blob.h"
namespace pdv { class Alchemy:public Blob{public:BlobKind kind()const override{return BlobKind::Alchemy;}void seed(int cell,int amount)override;bool transmute(int cell);protected:void evolve()override;private:int pos_=-1;}; }
