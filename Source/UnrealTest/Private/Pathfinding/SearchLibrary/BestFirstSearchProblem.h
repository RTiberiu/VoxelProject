#pragma once

#include <list>
#include "SearchProblem.h"
#include "Node.h"
#include "Path.h"
#include "ActionStatePair.h"

namespace aips {
    namespace search {
        namespace informed {

            class BestFirstSearchProblem {
            public:
                BestFirstSearchProblem(State* start, State* goal);

                Path* search();

                virtual double evaluation(const aips::search::Node& node) const = 0;

            protected:
                void addChildBinary(std::list<Node*>& fringe, Node* childNode);
                void addChildBinary(std::list<Node*>& fringe, Node* node, int left, int right);

                bool isGoal(State* state);
                Path* constructPath(Node* node);

            private:
                State* goalState;
                State* startState;
                int nodeVisited;
            };

        } // namespace informed
    } // namespace search
} // namespace aips

