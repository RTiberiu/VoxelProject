#pragma once

#include <string>
#include "Action.h"

class VoxelSearchState;

/**
    * This class defines an action-state pair.
    *
    * @author K. Hui
    * Translated from Java to C++ by Tiberiu Rociu
    */
class ActionStatePair {
public:
    /**
        * The action part of this pair.
        * This should be a customised sub-class object of Action, that models an action in your domain.
        */
    Action* action;

    /**
        * The state after the action is applied to the current state.
        * This should be a customised object that implements the State interface, modelling the problem state in your domain.
        */
    VoxelSearchState* state;

    /**
        * Creates an action-state object.
        *
        * @param action The action component of the pair.
        * @param state The state component of the pair.
        */
    ActionStatePair(Action* action, VoxelSearchState* state) : action(action), state(state) {}

    /**
        * This convenient method converts an action-state pair into a String (for printing?).
        * @return A string representation of the action-state pair.
        */
    std::string toString() const;
};
