#pragma once

#include "..\..\Utils\Semaphore\FairSemaphore.h"

#include "CoreMinimal.h"
#include "VoxelObjectMeshData.h"
#include "Containers/Queue.h"
#include "Misc/ScopeLock.h"
#include "ChunkLocationData.generated.h"

struct FVoxelObjectLocationData {
    FVector ObjectPosition;
    FIntPoint ObjectWorldCoords;
};

UCLASS()
class  UChunkLocationData : public UObject {
    GENERATED_BODY()

public:
    UChunkLocationData();

    ~UChunkLocationData();

    bool getChunkToSpawnPosition(FVoxelObjectLocationData& OutLocation);
    bool getChunkToDestroyPosition(FIntPoint& OutPosition);

    bool getComputedMeshDataAndLocationData(FVoxelObjectLocationData& locationData, FVoxelObjectMeshData& meshData);
    bool isMeshWaitingToBeSpawned();

    void addChunksToSpawnPosition(const FVoxelObjectLocationData position);
    void addChunksToDestroyPosition(const FIntPoint& position);
    void addMeshDataForPosition(const FVoxelObjectLocationData chunkLocationData, const FVoxelObjectMeshData meshData);

    void emptyPositionQueues();

private:
    // Queue for storing chunks position that need to be spawned
    TQueue<FVoxelObjectLocationData> chunksToSpawnPositions;
    FairSemaphore* ChunksToSpawnSemaphore; // TODO might not be needed in a producer-consumer pattern, since TQueue is thread-safe

    // Queue for storing chunks position that need to be destroyed
    TQueue<FIntPoint> chunksToDestroyPositions;
    FairSemaphore* ChunksToDestroySemaphore; // TODO might not be needed in a producer-consumer pattern, since TQueue is thread-safe

    // Map for storing chunks mesh data
    TQueue<FVoxelObjectLocationData> locationDataForComputedMeshes;
    TQueue<FVoxelObjectMeshData> computedMeshData;

    FairSemaphore* MeshDataSemaphore;
};
