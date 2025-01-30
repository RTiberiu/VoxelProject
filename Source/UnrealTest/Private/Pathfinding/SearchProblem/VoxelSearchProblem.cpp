#include "VoxelSearchProblem.h"

VoxelSearchProblem::VoxelSearchProblem(VoxelSearchState& initialState, VoxelSearchState& goalState)
    : BestFirstSearchProblem(&initialState, &goalState),
    voxelSearchGoalState(&goalState),
    voxelSearchInitialState(&initialState) {
}

double VoxelSearchProblem::evaluation(const Node& node) const {
    return heuristic(*node.state) + node.getCost(); // f(n) = h(n) + g(n)
}

double VoxelSearchProblem::heuristic(const VoxelSearchState& state) const {
    return getManhattanDistanceCost(state);
}

bool VoxelSearchProblem::isGoal(const VoxelSearchState& state) const {
    return state.equals(voxelSearchGoalState);
}

double VoxelSearchProblem::getManhattanDistanceCost(const VoxelSearchState& currentState) const {
    const FVector& currentPosition = currentState.getPosition();
	const FVector& goalPosition = voxelSearchGoalState->getPosition();

    const float currentX = currentPosition.X;
    const float currentY = currentPosition.Y;
    const float goalX = goalPosition.X;
    const float goalY = goalPosition.Y;

    return abs(currentX - goalX) + abs(currentY - goalY);
}