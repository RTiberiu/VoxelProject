#pragma once

#include "..\..\Utils\Semaphore\FairSemaphore.h"

#include "CoreMinimal.h"
#include "..\Vegetation\Trees\Tree.h"
#include "..\DataStructures\VoxelObjectMeshData.h"
#include "..\DataStructures\VoxelObjectLocationData.h"
#include "Containers/Queue.h"
#include "..\..\NPC\BasicNPC\BasicNPC.h"
#include "Misc/ScopeLock.h"
#include "ChunkLocationData.generated.h"

class ABasicNPC;
class WorldTerrainSettings;

UCLASS()
class  UChunkLocationData : public UObject {
    GENERATED_BODY()

public:
    UChunkLocationData();

    ~UChunkLocationData();

    void SetWorldTerrainSettings(UWorldTerrainSettings* InWorldTerrainSettings);

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

    void AddVegetationChunkSpawnPosition(FIntPoint& chunkPosition);
    void AddTreeChunkSpawnPosition(FIntPoint& chunkPosition);
    void RemoveVegetationChunkSpawnPosition(FIntPoint& chunkPosition);
    void RemoveTreeChunkSpawnPosition(FIntPoint& chunkPosition);
    void CheckForSpawnPointsInRange();
    void CheckAndAddVegetationNotInRange(
        TQueue<UCustomProceduralMeshComponent*>* GrassActorsToRemove,
        TQueue<UCustomProceduralMeshComponent*>* FlowerActorsToRemove
    );

    void CheckAndAddTreesNotInRange(TQueue<ATree*>* TreeActorsToRemove);


    TArray<FVoxelObjectLocationData> getTreeSpawnPositions();
    TArray<FVoxelObjectLocationData> getGrassSpawnPosition();
    TArray<FVoxelObjectLocationData> getFlowerSpawnPosition();
    TArray<TPair<FVoxelObjectLocationData, AnimalType>> getNPCSpawnPosition();

    TArray<FVoxelObjectLocationData> getTreeSpawnPositionsInRange();
    TArray<FVoxelObjectLocationData> getGrassSpawnPositionInRange();
    TArray<FVoxelObjectLocationData> getFlowerSpawnPositionInRange();
    TArray<TPair<FVoxelObjectLocationData, AnimalType>> getNPCSpawnPositionInRange();

    // Methods to add single spawn points for vegetation and NPCs
    void addTreeSpawnPosition(const FVoxelObjectLocationData position);
    void addGrassSpawnPosition(const FVoxelObjectLocationData position);
    void addFlowerSpawnPosition(const FVoxelObjectLocationData position);
    void addNPCSpawnPosition(const TPair<FVoxelObjectLocationData, AnimalType> positionAndType);

    // Methods to add batch spawn points for vegetation and NPCs
    void addTreeSpawnPositions(const TArray<FVoxelObjectLocationData>& positions);
    void addGrassSpawnPositions(const TArray<FVoxelObjectLocationData>& positions);
    void addFlowerSpawnPositions(const TArray<FVoxelObjectLocationData>& positions);
    void addNPCSpawnPositions(const TArray<TPair<FVoxelObjectLocationData, AnimalType>>& positionsAndTypes);

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

    bool AddUnspawnedNpcToDestroy(ABasicNPC* InNpcToDestroy);
    bool GetUnspawnedNpcToDestroy(ABasicNPC* InNpcToDestroy);

	void AddSurfaceVoxelPointsForChunk(const FIntPoint& chunkPosition, const TArray<int>& voxelPoints, const TArray<FVector2D>& avoidPoints);
	void RemoveSurfaceVoxelPointsForChunk(const FIntPoint& chunkPosition);
    TMap<FIntPoint, TArray<int>> GetSurfaceVoxelPoints();

    bool IsSurfacePointValid(const double& X, const double& Z);

    bool IsLocationOccupied(const FVector& currentPosition, const FVector& nextPosition, ABasicNPC* npcAtLocation);
    void AddOccupiedVoxelPosition(const FVector& position, ABasicNPC* npcAtLocation);

private:
    UWorldTerrainSettings* WorldTerrainSettingsRef;
    UWorldTerrainSettings*& WTSR = WorldTerrainSettingsRef;

    const int chunkSize{ 62 }; 

    // Queue for storing chunks position that need to be spawned
    TQueue<FVoxelObjectLocationData> chunksToSpawnPositions;
    FairSemaphore* ChunksToSpawnSemaphore; // TODO might not be needed in a producer-consumer pattern, since TQueue is thread-safe

    // Queue for storing chunks position that need to be destroyed
    TQueue<FIntPoint> chunksToDestroyPositions;
    FairSemaphore* ChunksToDestroySemaphore; // TODO might not be needed in a producer-consumer pattern, since TQueue is thread-safe

    TQueue<ATree*> unspawnedTreesToDestroy;
    TQueue<UProceduralMeshComponent*> unspawnedGrassToDestroy;
    TQueue<UProceduralMeshComponent*> unspawnedFlowerToDestroy;
    TQueue<ABasicNPC*> unspawnedNpcsToDestroy;

    TQueue<FIntPoint> treesToDestroy;
    TQueue<FIntPoint> grassToDestroy;
    TQueue<FIntPoint> flowersToDestroy;
    TQueue<FIntPoint> npcsToDestroy;

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
    TMap<FIntPoint, TArray<TPair<FVoxelObjectLocationData, AnimalType>>> NPCSpawnPositions;

    // Map to store the current Chunk Points where Vegetation should spawn
    // Note: This holds a reference to the grassSpawnPositions/flowersSpawnPositions
    // and when clearing those maps, the references should also be cleared from this one.
    TMap<FIntPoint, TArray<FVoxelObjectLocationData>*> VegetationChunkSpawnPoints;
    TMap<FIntPoint, TArray<FVoxelObjectLocationData>*> TreeChunkSpawnPoints;
    FairSemaphore* VegetationChunkSemaphore;
    FairSemaphore* TreeChunkSemaphore;

    TMap<FIntPoint, TArray<FVoxelObjectLocationData>*> grassInRangeSpawnPositions;
    TMap<FIntPoint, TArray<FVoxelObjectLocationData>*> flowersInRangeSpawnPositions;
    TMap<FIntPoint, TArray<FVoxelObjectLocationData>*> treesInRangeSpawnPositions;



    FairSemaphore* MeshDataSemaphore;

	FairSemaphore* SurfaceVoxelPointsSemaphore;
    // 2D Array in a 1D Array representing the height of each surface voxel (not scaled to the chunk position in the world)
    // FIntPoint represents the tile of the chunk
    // TArray<int> represents a flatten 2D array of all the heights of each surface voxel in the chunk
    TMap<FIntPoint, TArray<int>> surfaceVoxelPoints;
    
    // Positions to avoid in the pathfinding (not scaled to the chunk position in the world)
    // FIntPoint represents the tile of the chunk
    // TArray<FVector2D> represents each 2D coordinate inside the chunk that is occupied (currently just a tree)
    TMap<FIntPoint, TArray<FVector2D>> surfaceAvoidPoints;

    // Map used for avoiding overlapping NPCs during pathfinding and movement
    TMap<FIntPoint, ABasicNPC*> OccupiedVoxels; 
};
