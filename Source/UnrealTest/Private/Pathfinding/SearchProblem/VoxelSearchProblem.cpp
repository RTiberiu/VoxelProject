#include "VoxelSearchProblem.h"

VoxelSearchProblem::VoxelSearchProblem(const aips::search::State& initialState, const aips::search::State& goalState)
    : aips::search::informed::BestFirstSearchProblem((aips::search::State*)&initialState, (aips::search::State*)&goalState),
    voxelSearchGoalState((VoxelSearchState*)&goalState),
    voxelSearchInitialState((VoxelSearchState*)&initialState) {
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
	const VoxelSearchState* voxelSearchState = (VoxelSearchState*)&currentState;

    const FVector& currentPosition = voxelSearchState->getPosition();
	const FVector& goalPosition = voxelSearchGoalState->getPosition();

    const float currentX = currentPosition.X;
    const float currentY = currentPosition.Y;
    const float goalX = goalPosition.X;
    const float goalY = goalPosition.Y;

    return abs(currentX - goalX) + abs(currentY - goalY);
}