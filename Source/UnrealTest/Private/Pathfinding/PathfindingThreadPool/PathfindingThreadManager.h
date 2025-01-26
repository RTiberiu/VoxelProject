#pragma once

// #include "..\..\Utils\Semaphore\FairSemaphore.h"
#include "Containers/Queue.h"
#include "CoreMinimal.h"
#include "PathfindingRunnable.h"
#include <UObject/Object.h>
#include "PathfindingThreadManager.generated.h"

class UChunkLocationData;

UCLASS()
class UPathfindingThreadManager : public UObject {
    GENERATED_BODY()

public:
    UPathfindingThreadManager(const FObjectInitializer& ObjectInitializer);

    UPathfindingThreadManager(UWorldTerrainSettings* InWorldTerrainSettings, UChunkLocationData* InChunkLocationData, const int& NumThreads);

    void SetWorldTerrainSettings(UWorldTerrainSettings* InWorldTerrainSettings);
    void SetChunkLocationData(UChunkLocationData* InChunkLocationData);

    // TODO Adds a task to the thread pool
    void AddPathfindingTask(FVector* startLocation, FVector* endLocation);

    // Shuts down the thread pool
    void Shutdown();

private:
    UWorldTerrainSettings* WorldTerrainSettingsRef;
    UWorldTerrainSettings*& WTSR = WorldTerrainSettingsRef;

    UChunkLocationData* ChunkLocationDataRef;
    UChunkLocationData*& CLDR = ChunkLocationDataRef;

    // Thread queue
	TArray<PathfindingRunnable*> RunnablePool;
    TArray<FRunnableThread*> ThreadPool;

    // Store running threads, to avoid checking all the threads in the pool
	TArray<PathfindingRunnable*> RunningRunnable;
	TArray<FRunnableThread*> RunningThreads;

    // Thread management
    bool threadPoolRunning;

};
