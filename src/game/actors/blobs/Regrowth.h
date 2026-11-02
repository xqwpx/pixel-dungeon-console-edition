#pragma once
#include "game/actors/blobs/Blob.h"
namespace pdv { class Regrowth:public Blob{public:BlobKind kind()const override{return BlobKind::Regrowth;}protected:void evolve()override;}; }
