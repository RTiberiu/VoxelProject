#include "..\SearchLibrary\State.h"

class VoxelSearchState : public aips::search::State {
public:
    // Constructor
    VoxelSearchState();

    // Override pure virtual methods from State
    std::string toString() const override;
    bool equals(const aips::search::State* state) const override; // Change parameter type to base class pointer
    std::size_t hashCode() const override;
    std::vector<aips::search::ActionStatePair*> successor() const override;
};
