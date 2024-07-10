#pragma once

#include "CoreMinimal.h"
#include "ChunkLocationData.generated.h"

USTRUCT()
struct FChunkLocationData {
	GENERATED_BODY()

public:
	FVector ChunkPosition;
	FIntPoint ChunkWorldCoords;

	static bool getChunkToSpawnPosition(FChunkLocationData& OutLocation);
	static bool getChunkToDestroyPosition(FIntPoint& OutPosition);
	static void addChunksToSpawnPosition(const FChunkLocationData& position);
	static void addChunksToDestroyPosition(const FIntPoint& position);
private:
	// Queue for storing chunks position that need to be spawned
	static TQueue<FChunkLocationData> chunksToSpawnPositions;
	static FCriticalSection chunksToSpawnMutex;

	// Queue for storing chunks position that need to be destroyed
	static TQueue<FIntPoint> chunksToDestroyPositions;
	static FCriticalSection chunksToDestroyMutex;
};
