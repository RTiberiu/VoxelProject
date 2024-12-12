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

bool UChunkLocationData::getChunkToSpawnPosition(FChunkLocationData& OutLocation) {
    return chunksToSpawnPositions.Dequeue(OutLocation);
}

bool UChunkLocationData::getChunkToDestroyPosition(FIntPoint& OutPosition) {
    return chunksToDestroyPositions.Dequeue(OutPosition);
}

bool UChunkLocationData::getComputedMeshDataAndLocationData(FChunkLocationData& locationData, FChunkMeshData& meshData) {
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

void UChunkLocationData::addChunksToSpawnPosition(const FChunkLocationData position) {
    chunksToSpawnPositions.Enqueue(position);
}

void UChunkLocationData::addChunksToDestroyPosition(const FIntPoint& position) {
    chunksToDestroyPositions.Enqueue(position);
}

void UChunkLocationData::addMeshDataForPosition(const FChunkLocationData chunkLocationData, const FChunkMeshData meshData) {
    MeshDataSemaphore->Acquire();
    locationDataForComputedMeshes.Enqueue(chunkLocationData);
    computedMeshData.Enqueue(meshData);
    MeshDataSemaphore->Release();
}

void UChunkLocationData::emptyPositionQueues() {
    chunksToSpawnPositions.Empty();
    chunksToDestroyPositions.Empty();
}

