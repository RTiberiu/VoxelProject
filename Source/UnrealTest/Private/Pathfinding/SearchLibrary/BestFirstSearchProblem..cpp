#include <iostream>
#include <unordered_map>
#include <list>
#include <vector>
#include "BestFirstSearchProblem.h"

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


namespace aips {
    namespace search {
        namespace informed {

            BestFirstSearchProblem::BestFirstSearchProblem(State* start, State* goal) : SearchProblem(start), goalState(goal) {}

            Path* BestFirstSearchProblem::search() {
                std::unordered_map<State*, Node*> visitedNodes; // history
                std::list<Node*> fringe; // the list of fringe nodes

                Node* rootNode = new Node(this->startState, nullptr, nullptr); // create root node
                fringe.push_back(rootNode); // add root node into fringe
                visitedNodes[rootNode->state] = rootNode; // seen root node and state
                this->nodeVisited++; // increment node count
                if (nodeVisited % 1000 == 0) // print message every 1000 nodes
                    std::cout << "No. of nodes explored: " << nodeVisited << std::endl;

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
                        if (nodeVisited % 1000 == 0) // print message every 1000 nodes
                            std::cout << "No. of nodes explored: " << nodeVisited << std::endl;

                        ActionStatePair* child = childrenNodes[i];
                        Action* action = child->action;
                        State* nextState = child->state;
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
                    double nodeValue = this->evaluation(node); // f(n) value of new node
                    if (left == right) { // left meets right
                        double leftValue = this->evaluation(*std::next(fringe.begin(), left)); // f(n) of node at position left
                        if (leftValue > nodeValue) {
                            auto it = fringe.begin();
                            std::advance(it, left);
                            fringe.insert(it, node); // new node goes before left
                            return;
                        }
                    }
                    int mid = (left + right) / 2;
                    double midValue = this->evaluation(*std::next(fringe.begin(), mid)); // f(n) of node at position mid
                    if (midValue > nodeValue) {
                        right = mid - 1;
                    } else {
                        left = mid + 1;
                    }
                }
            }

        } // namespace informed
    } // namespace search
} // namespace aips