#pragma once

#include "Pathfinding\SearchLibrary\BestFirstSearchProblem.h"
#include "VoxelSearchState.h"
#include "Pathfinding\SearchLibrary\Node.h"

class VoxelSearchProblem : public BestFirstSearchProblem {
public:
    VoxelSearchProblem(VoxelSearchState& initialState, VoxelSearchState& goalState);

    double evaluation(const Node& node) const override;
    double heuristic(VoxelSearchState& state) const;
    bool isGoal(VoxelSearchState& state) const;
    double getManhattanDistanceCost(VoxelSearchState& currentState) const;

private:
    VoxelSearchState* voxelSearchGoalState;
    VoxelSearchState* voxelSearchInitialState;
};
