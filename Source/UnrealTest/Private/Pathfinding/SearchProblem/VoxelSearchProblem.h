#pragma once

#include "..\SearchLibrary\Node.h"
#include "..\SearchLibrary\State.h"
#include "..\SearchLibrary\BestFirstSearchProblem.h"
#include "VoxelSearchState.h"

class VoxelSearchProblem : public BestFirstSearchProblem {
public:
    VoxelSearchProblem(VoxelSearchState& initialState, VoxelSearchState& goalState);

    double evaluation(const Node& node) const override;
    double heuristic(const VoxelSearchState& state) const;
    bool isGoal(const VoxelSearchState& state) const;
    double getManhattanDistanceCost(const VoxelSearchState& currentState) const;

private:
    //VoxelSearchState voxelSearchGoalState;
    const VoxelSearchState* voxelSearchGoalState;
    const VoxelSearchState* voxelSearchInitialState;
    double walkerWalkTime;
    int walkerAttentionCapacity;

public:

};
