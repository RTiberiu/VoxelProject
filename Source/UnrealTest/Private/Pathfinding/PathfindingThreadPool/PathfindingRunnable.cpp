#include "PathfindingRunnable.h"

PathfindingRunnable::PathfindingRunnable(UWorldTerrainSettings* InWorldTerrainSettingsRef, UChunkLocationData* InChunkLocationDataRef) {
	SetWorldTerrainSettings(InWorldTerrainSettingsRef);
	SetChunkLocationData(InChunkLocationDataRef);
}

PathfindingRunnable::~PathfindingRunnable() {
}

bool PathfindingRunnable::Init() {
	isRunning.AtomicSet(false);
	return false;
}

uint32 PathfindingRunnable::Run() {
	isRunning.AtomicSet(true);

	// TODO ADD HERE THE SEARCH


	isRunning.AtomicSet(false);
	return 0;
}

void PathfindingRunnable::Stop() {
}

void PathfindingRunnable::Exit() {
}

void PathfindingRunnable::SetSearchData(FVector* InStartLocation, FVector* InEndLocation) {
	startLocation = InStartLocation;
	endLocation = InEndLocation;
}

FThreadSafeBool PathfindingRunnable::IsRunning() const {
	return isRunning;
}

void PathfindingRunnable::SetWorldTerrainSettings(UWorldTerrainSettings* InWorldTerrainSettings) {
	WorldTerrainSettingsRef = InWorldTerrainSettings;
}

void PathfindingRunnable::SetChunkLocationData(UChunkLocationData* InChunkLocationData) {
	ChunkLocationDataRef = InChunkLocationData;
}
