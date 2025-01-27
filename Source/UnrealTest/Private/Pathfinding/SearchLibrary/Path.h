#pragma once

#include <iostream>
#include <iterator>
#include <list>
#include "ActionStatePair.h"

/**
* TODO Update this description
 * This class models a path that traces the route from a root node to a node in the tree.
 * It is simply a list of {@link ActionStatePair} objects plus a head node (of {@link State}).
 * Conceptually a path is a specialised list of action-state pairs with a head node.
 * That is why it is extending the {@link java.util.LinkedList} concrete class
 * which implements the {@link java.util.List} interface.
 * I chose the {@link java.util.LinkedList} class in this case.
 * It can be any class that implements {@link java.util.List}.
 *
 * You do not need to do with this class.
 * A {@link Path} is returned as the result when you do a search.
 * If the returned value is not <code>null</code>,
 * you can simply call the {@link Path#print() print()} method to print out the path.
 *
 * @author K. Hui
 * Translated from Java to C++ by Tiberiu Rociu
 *
 */

namespace aips {
    namespace search {

        class Path {
        public:
            State* head;
            double cost;

            Path() : head(nullptr), cost(0.0) {}

            /**
             * Prints the path, with each node and action.
             * The output is controlled by the toString() method
             * of the State objects and Action objects,
             * which can be customised in the domain specific sub-classes.
             */
            void print() const {
                if (head == nullptr) {
                    UE_LOG(LogTemp, Warning, TEXT("Head of path is nullptr."));
                    return;
                }
                UE_LOG(LogTemp, Warning, TEXT("%s"), *FString(head->toString().c_str()));
                for (const auto& next : path) {
                    UE_LOG(LogTemp, Warning, TEXT("%s"), *FString(next->action->toString().c_str()));
                    UE_LOG(LogTemp, Warning, TEXT("%s"), *FString(next->state->toString().c_str()));
                    UE_LOG(LogTemp, Warning, TEXT(""));
                }
            }
            std::list<ActionStatePair*> path;
        };

    } // namespace search
} // namespace aips

