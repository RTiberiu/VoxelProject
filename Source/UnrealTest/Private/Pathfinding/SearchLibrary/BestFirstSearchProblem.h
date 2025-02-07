#pragma once

#include <list>
#include "SearchProblem.h"
#include "Node.h"
#include "Path.h"
#include "..\SearchProblem\VoxelSearchState.h"
#include "ActionStatePair.h"

class BestFirstSearchProblem {
public:
    BestFirstSearchProblem(VoxelSearchState* start, VoxelSearchState* goal);

    Path* search();

    virtual double evaluation(const Node& node) const = 0;

    void StopSearching();

protected:
    void addChildBinary(std::list<Node*>& fringe, Node* childNode);
    void addChildBinary(std::list<Node*>& fringe, Node* node, int left, int right);

    bool isGoal(VoxelSearchState* state);
    Path* constructPath(Node* node);

private:
    VoxelSearchState* goalState;
    VoxelSearchState* startState;
    int nodeVisited;

    const int failSearchThreshold = 6000;

    FThreadSafeBool stopSearching;
};

