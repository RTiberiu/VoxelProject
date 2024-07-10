#include "ChunkLocationData.h"

TQueue<FChunkLocationData> FChunkLocationData::chunksToSpawnPositions;
FCriticalSection FChunkLocationData::chunksToSpawnMutex;

TQueue<FIntPoint> FChunkLocationData::chunksToDestroyPositions;
FCriticalSection FChunkLocationData::chunksToDestroyMutex;

bool FChunkLocationData::getChunkToSpawnPosition(FChunkLocationData& OutLocation) {
	FScopeLock Lock(&chunksToSpawnMutex);
	return chunksToSpawnPositions.Dequeue(OutLocation);
}

bool FChunkLocationData::getChunkToDestroyPosition(FIntPoint& OutPosition) {
	FScopeLock Lock(&chunksToDestroyMutex);
	return chunksToDestroyPositions.Dequeue(OutPosition);
}

void FChunkLocationData::addChunksToSpawnPosition(const FChunkLocationData& position) {
	FScopeLock Lock(&chunksToSpawnMutex);
	chunksToSpawnPositions.Enqueue(position);
}

void FChunkLocationData::addChunksToDestroyPosition(const FIntPoint& position) {
	FScopeLock Lock(&chunksToDestroyMutex);
	chunksToDestroyPositions.Enqueue(position);
}