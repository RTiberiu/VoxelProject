#pragma once

#include "..\SearchLibrary\Node.h"
#include "..\SearchLibrary\State.h"
#include "VoxelSearchState.h"

class VoxelSearchProblem {
private:
    //VoxelSearchState voxelSearchGoalState;
	const aips::search::State* voxelSearchGoalState;
    double walkerWalkTime;
    int walkerAttentionCapacity;

public:
    VoxelSearchProblem(const aips::search::State& initialState, const aips::search::State& goalState); 

    double evaluation(const aips::search::Node& node) const;
    double heuristic(const aips::search::State& state) const;
    bool isGoal(const aips::search::State& state) const;
    double getManhattanDistanceCost(const aips::search::State& currentState) const;
};
