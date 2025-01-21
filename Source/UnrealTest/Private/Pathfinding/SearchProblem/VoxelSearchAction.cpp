#include "VoxelSearchAction.h"

VoxelSearchAction::VoxelSearchAction(const FVector& InNewPosition) : newPosition(InNewPosition) {
}

std::string VoxelSearchAction::toString() const {
    return "Moving to the new position: " + std::string(TCHAR_TO_UTF8(*newPosition.ToString()));
}
