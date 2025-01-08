#include "ChunkLocationData.h"

UChunkLocationData::UChunkLocationData() : 
    ChunksToSpawnSemaphore(new FairSemaphore(1)),
    ChunksToDestroySemaphore(new FairSemaphore(1)),
    MeshDataSemaphore(new FairSemaphore(1)),
    TreesToSpawnSemaphore(new FairSemaphore(1)) {
}

UChunkLocationData::~UChunkLocationData() {
    // Clean up the semaphores
    delete ChunksToSpawnSemaphore;
    delete ChunksToDestroySemaphore;
    delete MeshDataSemaphore;
    delete TreesToSpawnSemaphore;
}

bool UChunkLocationData::getChunkToSpawnPosition(FVoxelObjectLocationData& OutLocation) {
    return chunksToSpawnPositions.Dequeue(OutLocation);
}

bool UChunkLocationData::getChunkToDestroyPosition(FIntPoint& OutPosition) {
    return chunksToDestroyPositions.Dequeue(OutPosition);
}

bool UChunkLocationData::getComputedMeshDataAndLocationData(FVoxelObjectLocationData& locationData, FVoxelObjectMeshData& meshData) {
    MeshDataSemaphore->Acquire();
    bool removedLocationData = locationDataForComputedMeshes.Dequeue(locationData);
    bool removedMeshData = computedMeshData.Dequeue(meshData);
    MeshDataSemaphore->Release();

    return removedLocationData && removedMeshData;
}

bool UChunkLocationData::isMeshWaitingToBeSpawned() {
    MeshDataSemaphore->Acquire();
    bool isMeshWaiting = !locationDataForComputedMeshes.IsEmpty();
    MeshDataSemaphore->Release();
    return isMeshWaiting;
}

void UChunkLocationData::addChunksToSpawnPosition(const FVoxelObjectLocationData position) {
    chunksToSpawnPositions.Enqueue(position);
}

void UChunkLocationData::addChunksToDestroyPosition(const FIntPoint& position) {
    if (position.X == 5 && position.Y == -3) {
        UE_LOG(LogTemp, Warning, TEXT("addChunksToDestroyPosition() -- Added the point X:5 Z:-3."));
    }
    chunksToDestroyPositions.Enqueue(position);
}

void UChunkLocationData::addMeshDataForPosition(const FVoxelObjectLocationData chunkLocationData, const FVoxelObjectMeshData meshData) {
    MeshDataSemaphore->Acquire();
    locationDataForComputedMeshes.Enqueue(chunkLocationData);
    computedMeshData.Enqueue(meshData);
    MeshDataSemaphore->Release();
}

void UChunkLocationData::addTreeToDestroyPosition(const FIntPoint& treePosition) {
    treesToDestroy.Enqueue(treePosition);
}

bool UChunkLocationData::getTreeToDestroyPosition(FIntPoint& treePosition) {
    return treesToDestroy.Dequeue(treePosition);
}

void UChunkLocationData::emptyPositionQueues() {
    chunksToSpawnPositions.Empty();
    chunksToDestroyPositions.Empty();
}

TArray<FVoxelObjectLocationData> UChunkLocationData::getTreeSpawnPositions() {
    TArray<FVoxelObjectLocationData> output;
    TreesToSpawnSemaphore->Acquire();
   
    // Get the first item from the map // TODO Might be worth replacing the map with a Queue
    if (!treesSpawnPositions.IsEmpty()) {
        for (const TPair<FIntPoint, TArray<FVoxelObjectLocationData>>& pair : treesSpawnPositions) {
            output = pair.Value; 
            treesSpawnPositions.Remove(pair.Key);
            break; 
        }
    }

    TreesToSpawnSemaphore->Release();
    return output;
}

bool UChunkLocationData::getGrassSpawnPosition(FVoxelObjectLocationData& OutLocation) {
    return grassSpawnPositions.Dequeue(OutLocation);
}

bool UChunkLocationData::getFlowerSpawnPosition(FVoxelObjectLocationData& OutLocation) {
    return flowersSpawnPositions.Dequeue(OutLocation);
}

void UChunkLocationData::addTreeSpawnPosition(const FVoxelObjectLocationData position) {
    TreesToSpawnSemaphore->Acquire();

    // If it exists, add the new tree position to the existing array
    if (treesSpawnPositions.Contains(position.ObjectWorldCoords)) {
        treesSpawnPositions[position.ObjectWorldCoords].Add(position);
    }
    else {
        // If not, create a new array with the new tree position
        treesSpawnPositions.Add(position.ObjectWorldCoords, TArray<FVoxelObjectLocationData>({ position }));
    }

    TreesToSpawnSemaphore->Release();
}

void UChunkLocationData::addGrassSpawnPosition(const FVoxelObjectLocationData position) {
    grassSpawnPositions.Enqueue(position);
}

void UChunkLocationData::addFlowerSpawnPosition(const FVoxelObjectLocationData position) {
    flowersSpawnPositions.Enqueue(position);
}

bool UChunkLocationData::isTreeWaitingToBeSpawned(const TArray<FIntPoint>& treeRadiusPoints) {
    TreesToSpawnSemaphore->Acquire();

    bool output = false;
    for (const FIntPoint& point : treeRadiusPoints) {
        if (treesSpawnPositions.Contains(point)) {
            output = true;
            break;
        }
    }
    TreesToSpawnSemaphore->Release();

    return output;

}

bool UChunkLocationData::isGrassWaitingToBeSpawned() {
    bool isGrassWaiting = !grassSpawnPositions.IsEmpty();
    return isGrassWaiting;
}

bool UChunkLocationData::isFlowerWaitingToBeSpawned() {
    bool isFlowerWaiting = !flowersSpawnPositions.IsEmpty();
    return isFlowerWaiting;
}

void UChunkLocationData::RemoveTreeSpawnPosition(const FIntPoint& point) {
    TreesToSpawnSemaphore->Acquire();

    // Iterating over the map and removing the key and value
    for (TMap<FIntPoint, TArray<FVoxelObjectLocationData>>::TIterator SpawnPosition(treesSpawnPositions); SpawnPosition; ++SpawnPosition) {
        if (SpawnPosition.Key() == point) {
            SpawnPosition.RemoveCurrent();
            break;  
        }
    }

    TreesToSpawnSemaphore->Release();
}

bool UChunkLocationData::isTreeWaitingToBeDestroyed() {
    bool isTreeWaiting = !treesToDestroy.IsEmpty();
    return isTreeWaiting;
}

bool UChunkLocationData::AddUnspawnedTreeToDestroy(ATree* treeToDestroy) {
    return unspawnedTreesToDestroy.Enqueue(treeToDestroy);
}

bool UChunkLocationData::GetUnspawnedTreeToDestroy(ATree* treeToDestroy) {
    return unspawnedTreesToDestroy.Dequeue(treeToDestroy);
}
