#pragma once
#include "game/actors/blobs/Blob.h"
namespace pdv { class Foliage:public Blob{public:BlobKind kind()const override{return BlobKind::Foliage;}protected:void evolve()override;}; }
