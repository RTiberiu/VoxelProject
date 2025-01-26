#include "PathfindingRunnable.h"
#include "PathfindingThreadManager.h"

UPathfindingThreadManager::UPathfindingThreadManager(const FObjectInitializer& ObjectInitializer) {
}

UPathfindingThreadManager::UPathfindingThreadManager(UWorldTerrainSettings* InWorldTerrainSettings, UChunkLocationData* InChunkLocationData, const int& NumThreads) {
    threadPoolRunning = true;

    SetWorldTerrainSettings(InWorldTerrainSettings);
    SetChunkLocationData(InChunkLocationData);

    for (int i = 0; i < NumThreads; i++) {
        PathfindingRunnable* PathfindingRunnable = new PathfindingRunnable(InWorldTerrainSettings, InChunkLocationData);
        FRunnableThread* Thread = FRunnableThread::Create(PathfindingRunnable, *FString::Printf(TEXT("PathfindingThread%d"), i));

        RunnablePool.Add(PathfindingRunnable);
        ThreadPool.Add(Thread);
    }
}

void UPathfindingThreadManager::SetWorldTerrainSettings(UWorldTerrainSettings* InWorldTerrainSettings) {
	WorldTerrainSettingsRef = InWorldTerrainSettings;
}

void UPathfindingThreadManager::SetChunkLocationData(UChunkLocationData* InChunkLocationData) {
	ChunkLocationDataRef = InChunkLocationData;
}

void UPathfindingThreadManager::AddPathfindingTask(FVector* startLocation, FVector* endLocation) {
    // TODO Add some wait mechanism or something to wait when there are no available threads
    if (RunnablePool.Num() != 0 && ThreadPool.Num() != 0) {

        // TODO This doesn't correctly start a thread. I need to adjust the logic here.

		// Remove available runnable from the pool and add it to the running threads
        PathfindingRunnable* runnable = RunnablePool[0];
        RunnablePool.RemoveAt(0);
        RunningRunnable.Add(runnable);

		FRunnableThread* thread = ThreadPool[0];
		ThreadPool.RemoveAt(0);
		RunningThreads.Add(thread);

        // Set the runnable search data and run
        runnable->SetSearchData(startLocation, endLocation);
        runnable->Run();

        // TODO Implement a flag system to add the thread back to the available pool

        // When runnable is done, add it back to the available pool
        RunningRunnable.Remove(runnable);
		RunnablePool.Add(runnable);

		RunningThreads.Remove(thread);
		ThreadPool.Add(thread);

    }
}

void UPathfindingThreadManager::Shutdown() {
	// TODO Shutdown all the threads and destroy the objects
}
