#pragma once

#include "..\SearchLibrary\Action.h"
#include <string>

class VoxelSearchAction : public Action {
public:
    VoxelSearchAction(const FVector& InNewPosition);
    std::string toString() const override;

private:
    const FVector newPosition;
};
