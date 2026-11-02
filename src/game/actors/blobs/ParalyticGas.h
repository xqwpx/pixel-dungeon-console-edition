#pragma once
#include "game/actors/blobs/Blob.h"

namespace pdv {

class ParalyticGas : public Blob {
public:
    BlobKind kind() const override {
        return BlobKind::ParalyticGas;
    }

protected:
    void evolve() override;
};

} // namespace pdv
