#include "VoxelSearchState.h"

// Constructor
VoxelSearchState::VoxelSearchState(const FVector& InPosition, UChunkLocationData* InChunkLocationDataRef) : position(InPosition), ChunkLocationDataRef(InChunkLocationDataRef) {
}

std::string VoxelSearchState::toString() const {
    return "VoxelSearchState: Position(" + std::to_string(position.X) + ", " + std::to_string(position.Y) + ", " + std::to_string(position.Z) + ")";
}

bool VoxelSearchState::equals(const VoxelSearchState* state) const {
    return state->position == this->position;
}

// Generates a hash code for the VoxelSearchState based on the position vector
std::size_t VoxelSearchState::hashCode() const {
    std::size_t seed = 0;
    std::hash<float> hasher;
    seed ^= hasher(position.X) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    seed ^= hasher(position.Y) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    seed ^= hasher(position.Z) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    return seed;
}

// Get all the possible positions that can be reached from the current position
std::vector<ActionStatePair*> VoxelSearchState::successor() const {
    
    std::vector<ActionStatePair*> successors;

	// Possible moves (left, right, up, down, up-left, up-right, down-left, down-right)
    std::vector<FVector> possibleMoves = {
        FVector(-1, 0, 0), FVector(1, 0, 0), 
        FVector(0, 1, 0), FVector(0, -1, 0), 
        FVector(-1, 1, 0), FVector(1, 1, 0),
        FVector(-1, -1, 0), FVector(1, -1, 0)
    };

    // Generate successors based on possible moves
    for (const FVector& move : possibleMoves) {
        FVector newPosition = position + move;

        // Validate position is not occupied by a solid object (currently just by a tree)
        bool isValid = CLDR->IsSurfacePointValid(newPosition.X, newPosition.Y);

        if (!isValid) {
            continue;
        }

        VoxelSearchAction* newAction = new VoxelSearchAction(newPosition);
        VoxelSearchState* newState = new VoxelSearchState(newPosition, CLDR);
        ActionStatePair* actionStatePair = new ActionStatePair(newAction, newState);
        successors.push_back(actionStatePair);
    }

    return successors;
}

const FVector& VoxelSearchState::getPosition() const {
    return position;
}

void VoxelSearchState::SetChunkLocationData(UChunkLocationData* InChunkLocationData) {
    ChunkLocationDataRef = InChunkLocationData;
}
