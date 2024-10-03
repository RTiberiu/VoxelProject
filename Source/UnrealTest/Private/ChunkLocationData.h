#pragma once

#include "FairSemaphore.h"

#include "CoreMinimal.h"
#include "ChunkMeshData.h"
#include "Containers/Queue.h"
#include "Misc/ScopeLock.h"
#include "ChunkLocationData.generated.h"

struct FChunkLocationData {
    FVector ChunkPosition;
    FIntPoint ChunkWorldCoords;
};

UCLASS()
class  UChunkLocationData : public UObject {
    GENERATED_BODY()

public:
    UChunkLocationData();

    ~UChunkLocationData();

    bool getChunkToSpawnPosition(FChunkLocationData& OutLocation);
    bool getChunkToDestroyPosition(FIntPoint& OutPosition);

    bool getComputedMeshDataAndLocationData(FChunkLocationData& locationData, FChunkMeshData& meshData);
    bool isMeshWaitingToBeSpawned();

    void addChunksToSpawnPosition(const FChunkLocationData position);
    void addChunksToDestroyPosition(const FIntPoint& position);
    void addMeshDataForPosition(const FChunkLocationData chunkLocationData, const FChunkMeshData meshData);

    void emptyPositionQueues();

private:
    // Queue for storing chunks position that need to be spawned
    TQueue<FChunkLocationData> chunksToSpawnPositions;
    FairSemaphore* ChunksToSpawnSemaphore; // TODO might not be needed in a producer-consumer pattern, since TQueue is thread-safe

    // Queue for storing chunks position that need to be destroyed
    TQueue<FIntPoint> chunksToDestroyPositions;
    FairSemaphore* ChunksToDestroySemaphore; // TODO might not be needed in a producer-consumer pattern, since TQueue is thread-safe

    // Map for storing chunks mesh data
    TQueue<FChunkLocationData> locationDataForComputedMeshes;
    TQueue<FChunkMeshData> computedMeshData;

    FairSemaphore* MeshDataSemaphore;
};
