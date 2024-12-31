#pragma once

#include "..\..\Utils\Semaphore\FairSemaphore.h"

#include "CoreMinimal.h"
#include "..\Vegetation\Trees\Tree.h"
#include "..\DataStructures\VoxelObjectMeshData.h"
#include "..\DataStructures\VoxelObjectLocationData.h"
#include "Containers/Queue.h"
#include "Misc/ScopeLock.h"
#include "ChunkLocationData.generated.h"


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

    void addTreeToDestroyPosition(const FIntPoint& tree);
    bool getTreeToDestroyPosition(FIntPoint& tree);
    
    void emptyPositionQueues();

    bool getTreeSpawnPosition(FVoxelObjectLocationData& OutLocation);
    bool getGrassSpawnPosition(FVoxelObjectLocationData& OutLocation);
    bool getFlowerSpawnPosition(FVoxelObjectLocationData& OutLocation);

    void addTreeSpawnPosition(const FVoxelObjectLocationData position);
    void addGrassSpawnPosition(const FVoxelObjectLocationData position);
    void addFlowerSpawnPosition(const FVoxelObjectLocationData position);

    bool isTreeWaitingToBeSpawned();
    bool isGrassWaitingToBeSpawned();
    bool isFlowerWaitingToBeSpawned();

    bool isTreeWaitingToBeDestroyed();


private:
    // Queue for storing chunks position that need to be spawned
    TQueue<FVoxelObjectLocationData> chunksToSpawnPositions;
    FairSemaphore* ChunksToSpawnSemaphore; // TODO might not be needed in a producer-consumer pattern, since TQueue is thread-safe

    // Queue for storing chunks position that need to be destroyed
    TQueue<FIntPoint> chunksToDestroyPositions;
    FairSemaphore* ChunksToDestroySemaphore; // TODO might not be needed in a producer-consumer pattern, since TQueue is thread-safe

    TQueue<FIntPoint> treesToDestroy;
    TQueue<FIntPoint> grassToDestroy;
    TQueue<FIntPoint> flowersToDestroy;

    // Queue for storing chunks mesh data
    TQueue<FVoxelObjectLocationData> locationDataForComputedMeshes;
    TQueue<FVoxelObjectMeshData> computedMeshData;

    // Queue for storing vegetation spawn points data
    TQueue<FVoxelObjectLocationData> treesSpawnPositions;
    TQueue<FVoxelObjectLocationData> grassSpawnPositions;
    TQueue<FVoxelObjectLocationData> flowersSpawnPositions;


    FairSemaphore* MeshDataSemaphore;
};
