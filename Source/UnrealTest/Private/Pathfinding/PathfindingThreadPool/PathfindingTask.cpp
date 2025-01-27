#include "PathfindingTask.h"
#include "Misc/DateTime.h"

FPathfindingTask::FPathfindingTask(
	const FVector& InStartLocation,
	const FVector& InEndLocation,
	UWorldTerrainSettings* InWorldTerrainSettingsRef,
	UChunkLocationData* InChunkLocationDataRef
) : StartLocation(InStartLocation), EndLocation(InEndLocation), WorldTerrainSettingsRef(InWorldTerrainSettingsRef), ChunkLocationDataRef(InChunkLocationDataRef) {

}

FPathfindingTask::~FPathfindingTask() {
    WorldTerrainSettingsRef = nullptr;
    ChunkLocationDataRef = nullptr;
}

void FPathfindingTask::DoThreadedWork() {
	// TestFakeSearch();

	aips::search::Path* path = GetPathToEndLocation();

	AdjustPathWithActualVoxelHeights(path);
}

void FPathfindingTask::Abandon() {
	// TODO IMPLEMENT THIS AND FLAG THE SEARCH TO STOP IF IT'S RUNNING
	UE_LOG(LogTemp, Warning, TEXT("Task abandoned for pathfinding from %s to %s"), *StartLocation.ToString(), *EndLocation.ToString());
}

void FPathfindingTask::SetWorldTerrainSettings(UWorldTerrainSettings* InWorldTerrainSettings) {
	WorldTerrainSettingsRef = InWorldTerrainSettings;
}

void FPathfindingTask::SetChunkLocationData(UChunkLocationData* InChunkLocationData) {
	ChunkLocationDataRef = InChunkLocationData;
}

void FPathfindingTask::TestFakeSearch() {
	// Perform pathfinding between startLocation and endLocation
	UE_LOG(LogTemp, Warning, TEXT("Started pathfinding for %s to %s"), *StartLocation.ToString(), *EndLocation.ToString());
	FPlatformProcess::Sleep(5.0f);
	UE_LOG(LogTemp, Warning, TEXT("Finished pathfinding for %s to %s"), *StartLocation.ToString(), *EndLocation.ToString());

	FPlatformProcess::Sleep(0.01f); // Avoiding tight loops
}

aips::search::Path* FPathfindingTask::GetPathToEndLocation() {
    // UE_LOG(LogTemp, Warning, TEXT("Started pathfinding for %s to %s"), *StartLocation.ToString(), *EndLocation.ToString());
    FDateTime StartTime = FDateTime::Now();
		
    VoxelSearchState startPosition = VoxelSearchState(StartLocation);
    VoxelSearchState endPosition = VoxelSearchState(EndLocation);

    VoxelSearchProblem* searchProblem = new VoxelSearchProblem(startPosition, endPosition);

    aips::search::Path* pathToGoal = searchProblem->search();

    FDateTime EndTime = FDateTime::Now();
    // pathToGoal->print(); // TESTING THE PATH

   /* FTimespan Duration = EndTime - StartTime;
    UE_LOG(LogTemp, Warning, TEXT("Finished pathfinding for %s to %s in %d minutes, %d seconds, and %f milliseconds"), 
        *StartLocation.ToString(), *EndLocation.ToString(), Duration.GetMinutes(), Duration.GetSeconds(), Duration.GetTotalMilliseconds());*/

    return pathToGoal;
}

void FPathfindingTask::AdjustPathWithActualVoxelHeights(aips::search::Path* path) {
	// TODO Implement this
}
