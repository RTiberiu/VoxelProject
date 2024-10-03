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

FChunkMeshData UChunkLocationData::getMeshDataForLocationData(const FChunkLocationData& locationData) {
    MeshDataSemaphore->Acquire();
    FChunkMeshData meshData = meshDataForChunkPosition.FindAndRemoveChecked(locationData);
    MeshDataSemaphore->Release();
    return meshData;
}

FChunkLocationData UChunkLocationData::getLocationDataForWaitingMesh() {
    MeshDataSemaphore->Acquire();

    FChunkLocationData locationData;
    for (const TPair<FChunkLocationData, FChunkMeshData>& pair : meshDataForChunkPosition) {
        locationData = pair.Key;
    }

    MeshDataSemaphore->Release();

    return locationData;
}

bool UChunkLocationData::isMeshWaitingToBeSpawned() {
    MeshDataSemaphore->Acquire();
    bool isMeshWaiting = meshDataForChunkPosition.Num() != 0;
    MeshDataSemaphore->Release();
    return isMeshWaiting;
}

void UChunkLocationData::addChunksToSpawnPosition(const FChunkLocationData& position) {
    chunksToSpawnPositions.Enqueue(position);
}

void UChunkLocationData::addChunksToDestroyPosition(const FIntPoint& position) {
    chunksToDestroyPositions.Enqueue(position);
}

void UChunkLocationData::addMeshDataForPosition(const FChunkLocationData& chunkLocationData, const FChunkMeshData& meshData) {
    MeshDataSemaphore->Acquire();
    meshDataForChunkPosition.Add(chunkLocationData, meshData);
    MeshDataSemaphore->Release();
}

void UChunkLocationData::emptyPositionQueues() {
    chunksToSpawnPositions.Empty();
    chunksToDestroyPositions.Empty();
}

