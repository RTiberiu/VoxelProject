//#pragma once
//
//#include <string>
//#include <vector>
//
//namespace aips {
//    namespace search {
//        class ActionStatePair;
//    }
//}
//
//namespace aips {
//    namespace search {
//
//        /**
//         * This interface defines the methods required on a problem state.
//         * Any class that represents a problem state must implement this interface.
//         *
//         * @author K. Hui
//         * Translated from Java to C++ by Tiberiu Rociu
//         */
//        class State {
//        public:
//            /**
//             * Return the state into a printable string.
//             * You should customise this method for your state so that it returns a string
//             * which can be easily printed in the console.
//             * @return A string representation of the state.
//             */
//            virtual std::string toString() const = 0;
//
//            /**
//             * Checks if two states are equal.
//             * This method is used when we check for repeated states in the history,
//             * and see if we have reached a goal state.
//             *
//             * As our problem states are represented by objects,
//             * they may be 2 different State objects but with the same attribute values.
//             *
//             * Two states are considered "equal" if they have the same attribute values.
//             * e.g. the case of 2 twins.
//             *
//             * If you get this wrong for your State class, the search may fall into an infinite loop (as it cannot see a repeated state),
//             * or fail to recognise a goal when it reaches one.
//             *
//             * @param state The other state to compare with.
//             * @return true if the current state and the other are the same (value-wise). false otherwise.
//             */
//            virtual bool equals(const State* state) const = 0;
//
//            /**
//             * Calculate the hash code of a state.
//             * @return The hash code of the state.
//             */
//            virtual std::size_t hashCode() const = 0;
//
//            /**
//             * Get the successors of the current state.
//             * @return A vector of ActionStatePair representing the successors.
//             */
//            virtual std::vector<ActionStatePair*> successor() const = 0;
//
//            virtual ~State() = default;
//        };
//
//    } // namespace search
//} // namespace aips
//
