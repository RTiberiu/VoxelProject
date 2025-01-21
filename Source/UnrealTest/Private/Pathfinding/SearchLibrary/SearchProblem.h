#pragma once

#include <unordered_set>
#include <list>
#include "State.h"
#include "Node.h"
#include "Path.h"
#include "ActionStatePair.h"

/**
 * This class defines the default behaviour of a search problem.
 * Any class modelling a domain-specific search problem must extend this class directly or indirectly.
 *
 * @author K. Hui
 * Translated from Java to C++ by Tiberiu Rociu
 *
 */

namespace aips {
    namespace search {

        /**
         * This class defines a search problem.
         */
        class SearchProblem {
        protected:
            State* startState; // the initial state
            int nodeVisited;

        public:
            /**
             * Create a SearchProblem instance with an initial state.
             * @param start The initial state of the search problem.
             */
            SearchProblem(State* start) : startState(start), nodeVisited(0) {}

            /**
             * Perform a search.
             */
            virtual Path* search() = 0;

            virtual bool isGoal(State* state) const = 0;
            virtual Path* constructPath(Node* node) const = 0;
            virtual void addChildrenNodes(std::list<Node*>& fringe, Node* node, const std::vector<ActionStatePair*>& childrenNodes) const = 0;
        };

    } // namespace search
} // namespace aips

