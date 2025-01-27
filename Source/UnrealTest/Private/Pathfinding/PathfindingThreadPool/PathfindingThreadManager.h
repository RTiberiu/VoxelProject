#pragma once

#include "Containers/Queue.h"
#include "CoreMinimal.h"
#include "PathfindingTask.h"
#include "Misc/QueuedThreadPool.h"

class UChunkLocationData;
class UWorldTerrainSettings;

class PathfindingThreadManager  {

public:
    PathfindingThreadManager(UWorldTerrainSettings* InWorldTerrainSettings, UChunkLocationData* InChunkLocationData, const int& NumThreads);

    void ShutDownThreadPool();
    
    void SetWorldTerrainSettings(UWorldTerrainSettings* InWorldTerrainSettings);
    void SetChunkLocationData(UChunkLocationData* InChunkLocationData);

    // TODO Adds a task to the thread pool
    void AddPathfindingTask(FVector& startLocation, FVector& endLocation);

    
private:
    UWorldTerrainSettings* WorldTerrainSettingsRef;
    UWorldTerrainSettings*& WTSR = WorldTerrainSettingsRef;

    UChunkLocationData* ChunkLocationDataRef;
    UChunkLocationData*& CLDR = ChunkLocationDataRef;

    FQueuedThreadPool* PathfindingThreadPool;

    // Thread queue
 //   TArray<FRunnableThread*> ThreadPool;
 //   TArray<PathfindingRunnable*> Runnables;

 //   // Pathfinding work queue
	//TQueue<IQueuedWork*> QueuedWork;

    // Thread management
    bool threadPoolRunning;

};
