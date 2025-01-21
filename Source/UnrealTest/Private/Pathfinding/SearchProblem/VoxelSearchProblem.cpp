#include "VoxelSearchProblem.h"

VoxelSearchProblem::VoxelSearchProblem(const aips::search::State& initialState, const aips::search::State& goalState)
    : voxelSearchGoalState(&goalState) {
}

double VoxelSearchProblem::evaluation(const aips::search::Node& node) const {
    return heuristic(*node.state) + node.getCost(); // f(n) = h(n) + g(n)
}

double VoxelSearchProblem::heuristic(const aips::search::State& state) const {
    return getManhattanDistanceCost(state);
}

bool VoxelSearchProblem::isGoal(const aips::search::State& state) const {
    return state.equals(voxelSearchGoalState);
}

double VoxelSearchProblem::getManhattanDistanceCost(const aips::search::State& currentState) const {
    // Implementation needed
    return 0.0;
}