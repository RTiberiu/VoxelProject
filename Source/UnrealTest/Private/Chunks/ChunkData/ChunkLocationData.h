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

    TArray<FVoxelObjectLocationData> getTreeSpawnPositions();
    TArray<FVoxelObjectLocationData> getGrassSpawnPosition();
    TArray<FVoxelObjectLocationData> getFlowerSpawnPosition();
    TArray<FVoxelObjectLocationData> getNPCSpawnPosition();

    void addTreeSpawnPosition(const FVoxelObjectLocationData position);
    void addGrassSpawnPosition(const FVoxelObjectLocationData position);
    void addFlowerSpawnPosition(const FVoxelObjectLocationData position);
    void addNPCSpawnPosition(const FVoxelObjectLocationData position);

    void RemoveTreeSpawnPosition(const FIntPoint& point);
    void RemoveGrassSpawnPosition(const FIntPoint& point);
    void RemoveFlowerSpawnPosition(const FIntPoint& point);
    void RemoveNPCSpawnPosition(const FIntPoint& point);

    bool AddUnspawnedTreeToDestroy(ATree* InTreeToDestroy);
    bool GetUnspawnedTreeToDestroy(ATree* InTreeToDestroy);

    bool AddUnspawnedGrassToDestroy(UProceduralMeshComponent* InGrassToDestroy);
    bool GetUnspawnedGrassToDestroy(UProceduralMeshComponent* InGrassToDestroy);

    bool AddUnspawnedFlowerToDestroy(UProceduralMeshComponent* InFlowerToDestroy);
    bool GetUnspawnedFlowerToDestroy(UProceduralMeshComponent* InFlowerToDestroy);

	void AddSurfaceVoxelPointsForChunk(const FIntPoint& chunkPosition, const TArray<int>& voxelPoints);
	void RemoveSurfaceVoxelPointsForChunk(const FIntPoint& chunkPosition);

private:
    // Queue for storing chunks position that need to be spawned
    TQueue<FVoxelObjectLocationData> chunksToSpawnPositions;
    FairSemaphore* ChunksToSpawnSemaphore; // TODO might not be needed in a producer-consumer pattern, since TQueue is thread-safe

    // Queue for storing chunks position that need to be destroyed
    TQueue<FIntPoint> chunksToDestroyPositions;
    FairSemaphore* ChunksToDestroySemaphore; // TODO might not be needed in a producer-consumer pattern, since TQueue is thread-safe

    TQueue<ATree*> unspawnedTreesToDestroy;
    TQueue<UProceduralMeshComponent*> unspawnedGrassToDestroy;
    TQueue<UProceduralMeshComponent*> unspawnedFlowerToDestroy;

    TQueue<FIntPoint> treesToDestroy;
    TQueue<FIntPoint> grassToDestroy;
    TQueue<FIntPoint> flowersToDestroy;

    // Queue for storing chunks mesh data
    TQueue<FVoxelObjectLocationData> locationDataForComputedMeshes;
    TQueue<FVoxelObjectMeshData> computedMeshData;

    // Queue for storing vegetation spawn points data
    FairSemaphore* TreesToSpawnSemaphore;
    TMap<FIntPoint, TArray<FVoxelObjectLocationData>> treesSpawnPositions;

    FairSemaphore* GrassToSpawnSemaphore;
    TMap<FIntPoint, TArray<FVoxelObjectLocationData>> grassSpawnPositions;

    FairSemaphore* FlowersToSpawnSemaphore;
    TMap<FIntPoint, TArray<FVoxelObjectLocationData>> flowersSpawnPositions;

    FairSemaphore* NPCToSpawnSemaphore;
    TMap<FIntPoint, TArray<FVoxelObjectLocationData>> NPCSpawnPositions;

    FairSemaphore* MeshDataSemaphore;

	FairSemaphore* SurfaceVoxelPointsSemaphore;
    TMap<FIntPoint, TArray<int>> surfaceVoxelPoints;
};
