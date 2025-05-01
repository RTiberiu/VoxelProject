#include "ActionStatePair.h"
#include "..\SearchProblem\VoxelSearchState.h" 

std::string ActionStatePair::toString() const {
    return action->toString() + "\n" + state->toString();
}
