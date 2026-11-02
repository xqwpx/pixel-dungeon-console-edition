#pragma once
#include "game/actors/blobs/Blob.h"
namespace pdv { class Web:public Blob{public:BlobKind kind()const override{return BlobKind::Web;}void seed(int cell,int amount)override;protected:void evolve()override;}; }
