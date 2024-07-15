#include "ChunkLocationData.h"

UChunkLocationData& UChunkLocationData::getInstance() {
    static UChunkLocationData instance;
    return instance;
}

bool UChunkLocationData::getChunkToSpawnPosition(FChunkLocationData& OutLocation) {
    FScopeLock Lock(&chunksToSpawnMutex);
    return chunksToSpawnPositions.Dequeue(OutLocation);
}

bool UChunkLocationData::getChunkToDestroyPosition(FIntPoint& OutPosition) {
    FScopeLock Lock(&chunksToDestroyMutex);
    return chunksToDestroyPositions.Dequeue(OutPosition);
}

void UChunkLocationData::addChunksToSpawnPosition(const FChunkLocationData& position) {
    FScopeLock Lock(&chunksToSpawnMutex);
    chunksToSpawnPositions.Enqueue(position);
}

void UChunkLocationData::addChunksToDestroyPosition(const FIntPoint& position) {
    FScopeLock Lock(&chunksToDestroyMutex);
    chunksToDestroyPositions.Enqueue(position);
}

