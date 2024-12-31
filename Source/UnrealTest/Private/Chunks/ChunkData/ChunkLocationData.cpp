#include "ChunkLocationData.h"

UChunkLocationData::UChunkLocationData() : 
    ChunksToSpawnSemaphore(new FairSemaphore(1)),
    ChunksToDestroySemaphore(new FairSemaphore(1)),
    MeshDataSemaphore(new FairSemaphore(1)) {
}

UChunkLocationData::~UChunkLocationData() {
    // Clean up the semaphores
    delete ChunksToSpawnSemaphore;
    delete ChunksToDestroySemaphore;
    delete MeshDataSemaphore;
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

bool UChunkLocationData::getTreeSpawnPosition(FVoxelObjectLocationData& OutLocation) {
    return treesSpawnPositions.Dequeue(OutLocation);
}

bool UChunkLocationData::getGrassSpawnPosition(FVoxelObjectLocationData& OutLocation) {
    return grassSpawnPositions.Dequeue(OutLocation);
}

bool UChunkLocationData::getFlowerSpawnPosition(FVoxelObjectLocationData& OutLocation) {
    return flowersSpawnPositions.Dequeue(OutLocation);
}

TArray<FVoxelObjectLocationData> UChunkLocationData::getTreesAtLocation(FIntPoint worldLocation) {
    TArray<FVoxelObjectLocationData> matchingTrees;
    TArray<FVoxelObjectLocationData> nonMatchingTrees;

    FVoxelObjectLocationData treeData;

    // Process the queue and separate the matching and non-matching items.
    while (treesSpawnPositions.Dequeue(treeData)) {
        if (treeData.ObjectWorldCoords == worldLocation) {
            matchingTrees.Add(treeData); // Add matching trees to the result array.
        } else {
            nonMatchingTrees.Add(treeData); // Store non-matching items.
        }
    }

    // Re-enqueue the non-matching items back into the original queue.
    for (const FVoxelObjectLocationData& nonMatchingTree : nonMatchingTrees) {
        treesSpawnPositions.Enqueue(nonMatchingTree);
    }

    return matchingTrees;
}

void UChunkLocationData::addTreeSpawnPosition(const FVoxelObjectLocationData position) {
    treesSpawnPositions.Enqueue(position);
}

void UChunkLocationData::addGrassSpawnPosition(const FVoxelObjectLocationData position) {
    grassSpawnPositions.Enqueue(position);
}

void UChunkLocationData::addFlowerSpawnPosition(const FVoxelObjectLocationData position) {
    flowersSpawnPositions.Enqueue(position);
}

bool UChunkLocationData::isTreeWaitingToBeSpawned() {
    bool isTreeWaiting = !treesSpawnPositions.IsEmpty();
    return isTreeWaiting;
}

bool UChunkLocationData::isGrassWaitingToBeSpawned() {
    bool isGrassWaiting = !grassSpawnPositions.IsEmpty();
    return isGrassWaiting;
}

bool UChunkLocationData::isFlowerWaitingToBeSpawned() {
    bool isFlowerWaiting = !flowersSpawnPositions.IsEmpty();
    return isFlowerWaiting;
}

bool UChunkLocationData::isTreeWaitingToBeDestroyed() {
    bool isTreeWaiting = !treesToDestroy.IsEmpty();
    return isTreeWaiting;
}
