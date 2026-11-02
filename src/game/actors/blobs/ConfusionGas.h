#pragma once
#include "game/actors/blobs/Blob.h"
namespace pdv { class ConfusionGas:public Blob{public:BlobKind kind()const override{return BlobKind::ConfusionGas;}protected:void evolve()override;}; }
