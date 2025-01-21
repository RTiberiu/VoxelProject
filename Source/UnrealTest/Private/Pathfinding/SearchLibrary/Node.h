#pragma once

#include "Action.h"

namespace aips {
    namespace search {
        class State;
    }
}

namespace aips {
    namespace search {

        class Node {
        public:
            State* state;
            Node* parent;
            Action* action;

            Node(State* state, Node* parent, Action* action)
                : state(state), parent(parent), action(action) {
            }

            /**
             * Return the depth of this node.
             * @return The depth of the node from root. The root node has a depth of 0.
             */
            int getDepth() const {
                // iterative version to remove tail recursion
                const Node* current = this;
                int result = 0;
                while (current->parent != nullptr) {
                    result++;
                    current = current->parent;
                }
                return result;
            }

            /**
             * Calculate the path cost from the root to this node.
             * @return The path cost.
             */
            double getCost() const {
                double result = 0.0;
                const Node* current = this;
                while (current->parent != nullptr) {
                    result += current->action->cost; // bug fix. (2021-03-01)
                    current = current->parent;       // move to parent
                }
                return result;
            }
        };

    } // namespace search
} // namespace aips

