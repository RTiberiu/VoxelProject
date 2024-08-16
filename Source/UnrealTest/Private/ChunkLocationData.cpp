#include "ChunkLocationData.h"

UChunkLocationData::UChunkLocationData() : 
    ChunksToSpawnSemaphore(new FairSemaphore(1)),
    ChunksToDestroySemaphore(new FairSemaphore(1)) { 
}

UChunkLocationData::~UChunkLocationData() {
    // Clean up the semaphores
    delete ChunksToSpawnSemaphore;
    delete ChunksToDestroySemaphore;
}

bool UChunkLocationData::getChunkToSpawnPosition(FChunkLocationData& OutLocation) {
    return chunksToSpawnPositions.Dequeue(OutLocation);
}

bool UChunkLocationData::getChunkToDestroyPosition(FIntPoint& OutPosition) {
    return chunksToDestroyPositions.Dequeue(OutPosition);
}

void UChunkLocationData::addChunksToSpawnPosition(const FChunkLocationData& position) {
    chunksToSpawnPositions.Enqueue(position);
}

void UChunkLocationData::addChunksToDestroyPosition(const FIntPoint& position) {
    chunksToDestroyPositions.Enqueue(position);
}

void UChunkLocationData::emptyPositionQueues() {
    chunksToSpawnPositions.Empty();
    chunksToDestroyPositions.Empty();
}

