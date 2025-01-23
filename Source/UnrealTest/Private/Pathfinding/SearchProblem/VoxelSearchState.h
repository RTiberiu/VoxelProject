#pragma once

#include "..\SearchLibrary\State.h"
#include "..\SearchLibrary\ActionStatePair.h"
#include "VoxelSearchAction.h"

class VoxelSearchState : public aips::search::State {
public:
    // Constructor
    VoxelSearchState(const FVector& InPosition);

    // Override pure virtual methods from State
    std::string toString() const override;
    bool equals(const aips::search::State* state) const override;
    std::size_t hashCode() const override;
    std::vector<aips::search::ActionStatePair*> successor() const override;

    const FVector& getPosition() const;
private:
    const FVector position;

};

