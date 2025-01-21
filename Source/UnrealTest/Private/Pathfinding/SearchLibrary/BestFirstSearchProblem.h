#pragma once

#include <list>
#include "SearchProblem.h"
#include "Node.h"
#include "Path.h"
#include "ActionStatePair.h"

namespace aips {
    namespace search {
        namespace informed {

            class BestFirstSearchProblem : public SearchProblem {
            public:
                BestFirstSearchProblem(State* start, State* goal);

                Path* search() override;

            protected:
                void addChildBinary(std::list<Node*>& fringe, Node* childNode);
                void addChildBinary(std::list<Node*>& fringe, Node* node, int left, int right);

            private:
                State* goalState;

                double evaluation(Node* node) const {
                    // Implement the evaluation function
                    return 0.0;
                }
            };

        } // namespace informed
    } // namespace search
} // namespace aips

