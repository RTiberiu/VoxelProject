#include "VoxelSearchState.h"

// Constructor
// TODO IMPLEMENT THIS
VoxelSearchState::VoxelSearchState() {
    // Initialize your state here
}

// Override pure virtual methods from State
// TODO IMPLEMENT THIS
std::string VoxelSearchState::toString() const {
    return "VoxelSearchState";
}

// TODO IMPLEMENT THIS
bool VoxelSearchState::equals(const aips::search::State* state) const {
    return false;
}

// TODO IMPLEMENT THIS
std::size_t VoxelSearchState::hashCode() const {
    return std::hash<std::string>()("VoxelSearchState");
}

// TODO IMPLEMENT THIS
std::vector<aips::search::ActionStatePair*> VoxelSearchState::successor() const {
    std::vector<aips::search::ActionStatePair*> successors;
    return successors;
}