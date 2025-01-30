#include "BestFirstSearchProblem.h"
#include <iostream>
#include <unordered_map>
#include <list>
#include <vector>

/**
*  TODO Update this description
 * This is a specialised {@link SearchProblem} abstract subclass for modelling best-first informed searches.
 * This class contains most ingredients of an informed search problem.
 *
 * To create your customised sub-class of informed search,
 * simply extend this class and then supply the evaluation function.
 *
 * @author K. Hui
 * Translated from Java to C++ by Tiberiu Rociu
 */

BestFirstSearchProblem::BestFirstSearchProblem(VoxelSearchState* start, VoxelSearchState* goal) : startState(start), goalState(goal), nodeVisited(0) {}

Path* BestFirstSearchProblem::search() {
    std::unordered_map<VoxelSearchState*, Node*> visitedNodes; // history
    std::list<Node*> fringe; // the list of fringe nodes

    Node* rootNode = new Node(this->startState, nullptr, nullptr); // create root node
    fringe.push_back(rootNode); // add root node into fringe
    visitedNodes[rootNode->state] = rootNode; // seen root node and state
    this->nodeVisited++; // increment node count
    if (nodeVisited % 3000 == 0) // print message every 3000 nodes
        UE_LOG(LogTemp, Warning, TEXT("No. of nodes explored: %d -- Trying to search from start state: %s to goal state: %s"),
            nodeVisited,
            *FString(startState->toString().c_str()),
            *FString(goalState->toString().c_str())
        );

    while (true) {
        if (fringe.empty()) // no more node to expand
            return nullptr; // no solution

        Node* node = fringe.front();
        fringe.pop_front(); // remove and take 1st node
        if (this->isGoal(node->state)) // if goal is found
            return constructPath(node); // construct path and return path

        std::vector<ActionStatePair*> childrenNodes = node->state->successor(); // get successors
        for (size_t i = 0; i < childrenNodes.size(); i++) {
            this->nodeVisited++; // increment node count
            if (nodeVisited % 3000 == 0) // print message every 3000 nodes
                UE_LOG(LogTemp, Warning, TEXT("No. of nodes explored: %d -- Trying to search from start state: %s to goal state: %s"),
                    nodeVisited,
                    *FString(startState->toString().c_str()),
                    *FString(goalState->toString().c_str())
                );

            ActionStatePair* child = childrenNodes[i];
            Action* action = child->action;
            VoxelSearchState* nextState = child->state;
            Node* lastSeenNode = visitedNodes[nextState]; // look up state in history

            if (lastSeenNode == nullptr) { // have not seen this state before
                Node* childNode = new Node(nextState, node, action); // create child node from state
                addChildBinary(fringe, childNode); // add child into fringe
                visitedNodes[nextState] = childNode; // add into history
            } else {
                // state is in history
            }
        }
    }
}

void BestFirstSearchProblem::addChildBinary(std::list<Node*>& fringe, Node* childNode) {
    addChildBinary(fringe, childNode, 0, fringe.size() - 1);
}

void BestFirstSearchProblem::addChildBinary(std::list<Node*>& fringe, Node* node, int left, int right) {
    while (true) {
        if (left > right) {
            auto it = fringe.begin();
            std::advance(it, left);
            fringe.insert(it, node);
            return;
        }
        double nodeValue = this->evaluation(*node); // f(n) value of new node
        if (left == right) { // left meets right
            double leftValue = this->evaluation(**std::next(fringe.begin(), left)); // f(n) of node at position left
            if (leftValue > nodeValue) {
                auto it = fringe.begin();
                std::advance(it, left);
                fringe.insert(it, node); // new node goes before left
                return;
            }
        }
        int mid = (left + right) / 2;
        double midValue = this->evaluation(**std::next(fringe.begin(), mid)); // f(n) of node at position mid
        if (midValue > nodeValue) {
            right = mid - 1;
        } else {
            left = mid + 1;
        }
    }
}

bool BestFirstSearchProblem::isGoal(VoxelSearchState* state) {
    return state->equals(goalState);
}

Path* BestFirstSearchProblem::constructPath(Node* node) {
    if (node == nullptr)
        return nullptr;

    Path* result = new Path();
    result->cost = node->getCost();
    while (node->parent != nullptr) {
        result->path.push_front(new ActionStatePair(node->action, node->state));	//add state to the beginning of list
        node = node->parent;
    }
    result->head = node->state;	//now node is the head of the path

    return result;
}
