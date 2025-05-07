#include "PathfindingThreadManager.h"

PathfindingThreadManager::PathfindingThreadManager(UWorldTerrainSettings* InWorldTerrainSettings, UChunkLocationData* InChunkLocationData, const int& NumThreads) {
	threadPoolRunning = true;

	SetWorldTerrainSettings(InWorldTerrainSettings);
	SetChunkLocationData(InChunkLocationData);

	// Alocate the thread pool and create the threads
	PathfindingThreadPool = FQueuedThreadPool::Allocate();
	bool threadPoolResult = PathfindingThreadPool->Create(NumThreads, 32 * 1024, EThreadPriority::TPri_Normal, TEXT("PathfindingThreadPool"));

	if (threadPoolResult) {
		UE_LOG(LogTemp, Warning, TEXT("Thread pool created with %d threads."), NumThreads);
	} else {
		UE_LOG(LogTemp, Error, TEXT("Failed to create FQueuedThreadPool!"));
		delete PathfindingThreadPool;
		PathfindingThreadPool = nullptr;
	}
}

void PathfindingThreadManager::ShutDownThreadPool() {
	if (PathfindingThreadPool) {
		UE_LOG(LogTemp, Warning, TEXT("Started the thread pool destroy process."));
		PathfindingThreadPool->Destroy();
		delete PathfindingThreadPool;
		PathfindingThreadPool = nullptr;
		UE_LOG(LogTemp, Warning, TEXT("Thread pool destroyed succesfully."));
	}
}

void PathfindingThreadManager::SetWorldTerrainSettings(UWorldTerrainSettings* InWorldTerrainSettings) {
	WorldTerrainSettingsRef = InWorldTerrainSettings;
}

void PathfindingThreadManager::SetChunkLocationData(UChunkLocationData* InChunkLocationData) {
	ChunkLocationDataRef = InChunkLocationData;
}

void PathfindingThreadManager::AddPathfindingTask(ABasicNPC* npcRef, FVector& startLocation, FVector& endLocation) {
	if (!PathfindingThreadPool) {
		UE_LOG(LogTemp, Error, TEXT("Thread pool is not initialized!"));
		return;
	}

	FPathfindingTask* NewTask = new FPathfindingTask(startLocation, endLocation, npcRef, WTSR, CLDR);
	PathfindingThreadPool->AddQueuedWork(NewTask);
}
