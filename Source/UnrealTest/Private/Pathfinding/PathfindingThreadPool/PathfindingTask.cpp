#include "PathfindingTask.h"
#include "..\..\Chunks\TerrainSettings\WorldTerrainSettings.h"
#include "Misc/DateTime.h"

FPathfindingTask::FPathfindingTask(
	const FVector& InStartLocation,
	const FVector& InEndLocation,
	ABasicNPC* InNPCRef,
	UWorldTerrainSettings* InWorldTerrainSettingsRef,
	UChunkLocationData* InChunkLocationDataRef
) : StartLocation(InStartLocation), EndLocation(InEndLocation), NPCRef(InNPCRef), WorldTerrainSettingsRef(InWorldTerrainSettingsRef), ChunkLocationDataRef(InChunkLocationDataRef), isSearching(false) {

}

FPathfindingTask::~FPathfindingTask() {
    WorldTerrainSettingsRef = nullptr;
    ChunkLocationDataRef = nullptr;
	NPCRef = nullptr;
	searchProblem = nullptr;
}

void FPathfindingTask::DoThreadedWork() {
	// TestFakeSearch();

	AdjustLocationsToUnrealScaling();

	Path* path = GetPathToEndLocation();

	AdjustPathWithActualVoxelHeights(path);

	// Notify the NPC that the path is ready
	NPCRef->SetPathToPlayerAndNotify(path);
}

void FPathfindingTask::Abandon() {
	// Notify the search problem to stop searching
	if (isSearching) {
		searchProblem->StopSearching();
	}
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

// Makes each unit in the start and end locaiton be the equivalent of a voxel size
void FPathfindingTask::AdjustLocationsToUnrealScaling() {
	StartLocation = FVector(FMath::FloorToDouble(StartLocation.X / WTSR->UnrealScale), FMath::FloorToDouble(StartLocation.Y / WTSR->UnrealScale), 0);
	EndLocation = FVector(FMath::FloorToDouble(EndLocation.X / WTSR->UnrealScale), FMath::FloorToDouble(EndLocation.Y / WTSR->UnrealScale), 0);
}

Path* FPathfindingTask::GetPathToEndLocation() {
    // UE_LOG(LogTemp, Warning, TEXT("Started pathfinding for %s to %s"), *StartLocation.ToString(), *EndLocation.ToString());
    FDateTime StartTime = FDateTime::Now();
	
    VoxelSearchState startPosition = VoxelSearchState(StartLocation, CLDR);
    VoxelSearchState endPosition = VoxelSearchState(EndLocation, CLDR);

    searchProblem = new VoxelSearchProblem(startPosition, endPosition);

	isSearching = true;
    Path* pathToGoal = searchProblem->search();
	isSearching = false;

    FDateTime EndTime = FDateTime::Now();
    // pathToGoal->print(); // TESTING THE PATH

   /* FTimespan Duration = EndTime - StartTime;
    UE_LOG(LogTemp, Warning, TEXT("Finished pathfinding for %s to %s in %d minutes, %d seconds, and %f milliseconds"), 
        *StartLocation.ToString(), *EndLocation.ToString(), Duration.GetMinutes(), Duration.GetSeconds(), Duration.GetTotalMilliseconds());*/

	// Cleanup 
	delete searchProblem;
	searchProblem = nullptr;

    return pathToGoal;
}

void FPathfindingTask::AdjustPathWithActualVoxelHeights(Path* path) {
	// Get actual surface voxel heights
	TMap<FIntPoint, TArray<int>> surfaceVoxelPoints = CLDR->GetSurfaceVoxelPoints();

	// Update each ActionStatePair in the path // TODO I HAVE TO VALIDATE THIS
	for (ActionStatePair* pair : path->path) {
		FVector& location = pair->state->getPosition();
		FIntPoint chunkPosition(FMath::FloorToInt(location.X / WTSR->UnrealScale), FMath::FloorToInt(location.Y / WTSR->UnrealScale));
		if (surfaceVoxelPoints.Contains(chunkPosition)) {
			const TArray<int>& heights = surfaceVoxelPoints[chunkPosition];

			int index = FMath::FloorToInt(location.X) + FMath::FloorToInt(location.Y) * WTSR->chunkSize;
			if (heights.IsValidIndex(index)) {
				location.Z = heights[index] * WTSR->UnrealScale;
				location.X = location.X * WTSR->UnrealScale;
				location.Y = location.Y * WTSR->UnrealScale;
			}
		}
	}
}
