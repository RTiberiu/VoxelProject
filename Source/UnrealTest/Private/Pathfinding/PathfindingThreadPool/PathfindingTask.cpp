#include "PathfindingTask.h"

FPathfindingTask::FPathfindingTask(
	const FVector& InStartLocation,
	const FVector& InEndLocation,
	UWorldTerrainSettings* InWorldTerrainSettingsRef,
	UChunkLocationData* InChunkLocationDataRef
) : StartLocation(InStartLocation), EndLocation(InEndLocation), WorldTerrainSettingsRef(InWorldTerrainSettingsRef), ChunkLocationDataRef(InChunkLocationDataRef) {

}

void FPathfindingTask::DoThreadedWork() {
	TestFakeSearch();
}

void FPathfindingTask::Abandon() {
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
