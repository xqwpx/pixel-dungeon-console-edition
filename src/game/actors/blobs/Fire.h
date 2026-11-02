#pragma once
#include "game/actors/blobs/Blob.h"

namespace pdv {

class Fire : public Blob {
public:
    BlobKind kind() const override {
        return BlobKind::Fire;
    }

    void seed(int cell, int amount) override;

protected:
    void evolve() override;

private:
    void burn(int pos);
};

} // namespace pdv
