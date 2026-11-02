#pragma once
#include "game/actors/blobs/Blob.h"
namespace pdv { class WellWater:public Blob{public:explicit WellWater(BlobKind kind):kind_(kind){}BlobKind kind()const override{return kind_;}void seed(int cell,int amount)override;bool affect(int cell);protected:void evolve()override;private:BlobKind kind_;int pos_=-1;}; }
