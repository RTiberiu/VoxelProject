#include "..\SearchProblem\VoxelSearchProblem.h"
#include <iostream>

int main() {

	FVector initialPosition(0, 0, 0);
    FVector goalPosition(7, 7, 0);

	VoxelSearchState initialState = VoxelSearchState(initialPosition);
	VoxelSearchState goalState = VoxelSearchState(goalPosition);

    VoxelSearchProblem* voxelSearchProblem = new VoxelSearchProblem(initialState, goalState);

	aips::search::Path* searchPath = voxelSearchProblem->search();

	if (searchPath != nullptr) {
		std::cout << "Path found!" << std::endl;
		searchPath->print();
	} else {
		std::cout << "Path not found!" << std::endl;
	}	

	


    return 0;
}
