#pragma once
#include "game/actors/blobs/Blob.h"

namespace pdv {

class ToxicGas : public Blob {
public:
    BlobKind kind() const override {
        return BlobKind::ToxicGas;
    }

protected:
    void evolve() override;
};

} // namespace pdv
