#pragma once

//#include "..\SearchLibrary\State.h"
#include "..\SearchLibrary\ActionStatePair.h"
#include "VoxelSearchAction.h"
#include "Chunks/ChunkData/ChunkLocationData.h"

class UChunkLocationData;

class VoxelSearchState {
public:
    // Constructor
    VoxelSearchState(const FVector& InPosition, UChunkLocationData* InChunkLocationDataRef);

    // Override pure virtual methods from State
    std::string toString() const;
    bool equals(const VoxelSearchState* state) const;
    std::size_t hashCode() const;
    std::vector<ActionStatePair*> successor() const;

    const FVector& getPosition() const;
private:
    UChunkLocationData* ChunkLocationDataRef;
    UChunkLocationData*& CLDR = ChunkLocationDataRef;
    void SetChunkLocationData(UChunkLocationData* InChunkLocationData);

    const FVector position;

};

