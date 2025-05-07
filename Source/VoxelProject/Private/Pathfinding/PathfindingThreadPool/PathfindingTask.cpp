#include "PathfindingTask.h"
#include "..\..\Chunks\TerrainSettings\WorldTerrainSettings.h"
#include "Misc/DateTime.h"
#include "..\..\Chunks\ChunkData\ChunkLocationData.h"


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
	AdjustLocationsToUnrealScaling();

	Path* path = GetPathToEndLocation();

	// Only adjust if path is not a nullptr
	if (path) {
		AdjustPathWithActualVoxelHeights(path);
	}

	// Notify the NPC that the path is ready
	NPCRef->SetPathToTargetAndNotify(path);
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

// Makes each unit in the start and end locaiton be the equivalent of a voxel size
void FPathfindingTask::AdjustLocationsToUnrealScaling() {
	StartLocation = FVector(FMath::FloorToDouble(StartLocation.X / WTSR->UnrealScale), FMath::FloorToDouble(StartLocation.Y / WTSR->UnrealScale), 0);
	EndLocation = FVector(FMath::FloorToDouble(EndLocation.X / WTSR->UnrealScale), FMath::FloorToDouble(EndLocation.Y / WTSR->UnrealScale), 0);
}

Path* FPathfindingTask::GetPathToEndLocation() {
    VoxelSearchState startPosition = VoxelSearchState(StartLocation, CLDR);
    VoxelSearchState endPosition = VoxelSearchState(EndLocation, CLDR);

    searchProblem = new VoxelSearchProblem(startPosition, endPosition);

	isSearching = true;
    Path* pathToGoal = searchProblem->search();
	isSearching = false;

	// Cleanup 
	delete searchProblem;
	searchProblem = nullptr;

    return pathToGoal;
}

void FPathfindingTask::AdjustPathWithActualVoxelHeights(Path* path) {
	// Get actual surface voxel heights
	TMap<FIntPoint, TArray<int>> surfaceVoxelPoints = CLDR->GetSurfaceVoxelPoints();

	// Update each ActionStatePair in the path
	for (ActionStatePair* pair : path->path) {
		FVector& location = pair->state->getPosition();


		FIntPoint chunkPosition(FMath::FloorToInt(location.X / WTSR->chunkSize), FMath::FloorToInt(location.Y / WTSR->chunkSize));
		
		if (surfaceVoxelPoints.Contains(chunkPosition)) {
			const TArray<int>& heights = surfaceVoxelPoints[chunkPosition];

			const int modX = FMath::Max(((static_cast<int>(location.X) % WTSR->chunkSize) + WTSR->chunkSize) % WTSR->chunkSize - 1, 0);
			const int modY = FMath::Max(((static_cast<int>(location.Y) % WTSR->chunkSize) + WTSR->chunkSize) % WTSR->chunkSize - 1, 0);

			const int index = modX * WTSR->chunkSize + modY;

			if (heights.IsValidIndex(index)) {
				location.Z = heights[index] * WTSR->UnrealScale;
				location.X = location.X * WTSR->UnrealScale + WTSR->HalfUnrealScale;
				location.Y = location.Y * WTSR->UnrealScale + WTSR->HalfUnrealScale;
			}
		}
	}
}

// Method used for testing. It prints all the heights in the current chunk, to better visualize their positions
void FPathfindingTask::PrintHeights(const TArray<int>& heights) {
	FString output;
	output += TEXT("CHUNK: \n");	

	for (int i = 0; i < heights.Num(); ++i) {
		output += FString::FromInt(heights[i]) + TEXT("\t");
		if ((i + 1) % 62 == 0) {
			output += TEXT("\n");
		}
	}
	UE_LOG(LogTemp, Warning, TEXT("%s"), *output);
}