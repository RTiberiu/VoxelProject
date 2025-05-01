#pragma once

//#include "..\SearchLibrary\State.h"
#include "..\SearchLibrary\ActionStatePair.h"
#include <vector>
//#include "VoxelSearchAction.h"
//#include "Chunks/ChunkData/ChunkLocationData.h"

class UChunkLocationData;

class VoxelSearchState {
public:
    // Constructor
    VoxelSearchState(FVector& InPosition, UChunkLocationData* InChunkLocationDataRef);

    // Override pure virtual methods from State
    std::string toString() const;
    bool equals(const VoxelSearchState* state) const;
    std::size_t hashCode() const;
    std::vector<ActionStatePair*> successor(VoxelSearchState* goalState) const;

    FVector& getPosition();
private:
    UChunkLocationData* ChunkLocationDataRef;
    UChunkLocationData*& CLDR = ChunkLocationDataRef;
    void SetChunkLocationData(UChunkLocationData* InChunkLocationData);

    FVector position;

};

