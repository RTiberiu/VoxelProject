#pragma once

#include "CoreMinimal.h"
#include "Containers/Queue.h"
#include "Misc/ScopeLock.h"
#include "ChunkLocationData.generated.h"

struct FChunkLocationData {
    FVector ChunkPosition;
    FIntPoint ChunkWorldCoords;
};

UCLASS()
class  UChunkLocationData : public UObject {
    GENERATED_BODY()

public:
    UChunkLocationData();

    bool getChunkToSpawnPosition(FChunkLocationData& OutLocation);
    bool getChunkToDestroyPosition(FIntPoint& OutPosition);
    void addChunksToSpawnPosition(const FChunkLocationData& position);
    void addChunksToDestroyPosition(const FIntPoint& position);

private:
    // Queue for storing chunks position that need to be spawned
    TQueue<FChunkLocationData> chunksToSpawnPositions;
    FCriticalSection chunksToSpawnMutex;

    // Queue for storing chunks position that need to be destroyed
    TQueue<FIntPoint> chunksToDestroyPositions;
    FCriticalSection chunksToDestroyMutex;
};
