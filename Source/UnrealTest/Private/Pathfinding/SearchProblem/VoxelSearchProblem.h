#pragma once

#include "..\SearchLibrary\Node.h"
#include "..\SearchLibrary\State.h"
#include "..\SearchLibrary\BestFirstSearchProblem.h"
#include "VoxelSearchState.h"

class VoxelSearchProblem : public aips::search::informed::BestFirstSearchProblem {
public:
    VoxelSearchProblem(const aips::search::State& initialState, const aips::search::State& goalState); 

    double evaluation(const aips::search::Node& node) const override;
    double heuristic(const aips::search::State& state) const;
    bool isGoal(const aips::search::State& state) const;
    double getManhattanDistanceCost(const aips::search::State& currentState) const;

private:
    //VoxelSearchState voxelSearchGoalState;
    const VoxelSearchState* voxelSearchGoalState;
    const VoxelSearchState* voxelSearchInitialState;
    double walkerWalkTime;
    int walkerAttentionCapacity;

public:

};
