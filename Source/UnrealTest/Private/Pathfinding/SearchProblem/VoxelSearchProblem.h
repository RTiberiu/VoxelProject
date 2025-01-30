#pragma once

#include "..\SearchLibrary\Node.h"
#include "..\SearchLibrary\State.h"
#include "..\SearchLibrary\BestFirstSearchProblem.h"
#include "VoxelSearchState.h"

class VoxelSearchProblem : public BestFirstSearchProblem {
public:
    VoxelSearchProblem(VoxelSearchState& initialState, VoxelSearchState& goalState);

    double evaluation(const Node& node) const override;
    double heuristic(VoxelSearchState& state) const;
    bool isGoal(VoxelSearchState& state) const;
    double getManhattanDistanceCost(VoxelSearchState& currentState) const;

private:
    //VoxelSearchState voxelSearchGoalState;
    VoxelSearchState* voxelSearchGoalState;
    VoxelSearchState* voxelSearchInitialState;
    double walkerWalkTime;
    int walkerAttentionCapacity;

public:

};
