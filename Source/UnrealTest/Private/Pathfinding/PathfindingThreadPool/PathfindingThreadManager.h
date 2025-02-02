#pragma once

#include "Containers/Queue.h"
#include "CoreMinimal.h"
#include "PathfindingTask.h"
//#include "..\..\NPC\BasicNPC\BasicNPC.h"
#include "Misc/QueuedThreadPool.h"

class ABasicNPC;
class UChunkLocationData;
class UWorldTerrainSettings;

class PathfindingThreadManager  {

public:
    PathfindingThreadManager(UWorldTerrainSettings* InWorldTerrainSettings, UChunkLocationData* InChunkLocationData, const int& NumThreads);

    void ShutDownThreadPool();
    
    void SetWorldTerrainSettings(UWorldTerrainSettings* InWorldTerrainSettings);
    void SetChunkLocationData(UChunkLocationData* InChunkLocationData);

    // TODO Adds a task to the thread pool
    void AddPathfindingTask(ABasicNPC* npcRef, FVector& startLocation, FVector& endLocation);

    
private:
    UWorldTerrainSettings* WorldTerrainSettingsRef;
    UWorldTerrainSettings*& WTSR = WorldTerrainSettingsRef;

    UChunkLocationData* ChunkLocationDataRef;
    UChunkLocationData*& CLDR = ChunkLocationDataRef;

    FQueuedThreadPool* PathfindingThreadPool;

    // Thread management
    bool threadPoolRunning;

};
