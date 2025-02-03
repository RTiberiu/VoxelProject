#include "ChunkLocationData.h"

UChunkLocationData::UChunkLocationData() :
	ChunksToSpawnSemaphore(new FairSemaphore(1)),
	ChunksToDestroySemaphore(new FairSemaphore(1)),
	MeshDataSemaphore(new FairSemaphore(1)),
	GrassToSpawnSemaphore(new FairSemaphore(1)),
	FlowersToSpawnSemaphore(new FairSemaphore(1)),
	NPCToSpawnSemaphore(new FairSemaphore(1)),
	TreesToSpawnSemaphore(new FairSemaphore(1)),
	SurfaceVoxelPointsSemaphore(new FairSemaphore(1)) {
}

UChunkLocationData::~UChunkLocationData() {
	// Clean up the semaphores
	delete ChunksToSpawnSemaphore;
	delete ChunksToDestroySemaphore;
	delete MeshDataSemaphore;
	delete TreesToSpawnSemaphore;
	delete FlowersToSpawnSemaphore;
	delete GrassToSpawnSemaphore;
	delete NPCToSpawnSemaphore;
	delete SurfaceVoxelPointsSemaphore;
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

TArray<FVoxelObjectLocationData> UChunkLocationData::getGrassSpawnPosition() {
	TArray<FVoxelObjectLocationData> output;
	GrassToSpawnSemaphore->Acquire();

	// Get the first item from the map // TODO Might be worth replacing the map with a Queue
	if (!grassSpawnPositions.IsEmpty()) {
		for (const TPair<FIntPoint, TArray<FVoxelObjectLocationData>>& pair : grassSpawnPositions) {
			output = pair.Value;
			grassSpawnPositions.Remove(pair.Key);
			break;
		}
	}

	GrassToSpawnSemaphore->Release();
	return output;
}

TArray<FVoxelObjectLocationData> UChunkLocationData::getFlowerSpawnPosition() {
	TArray<FVoxelObjectLocationData> output;
	FlowersToSpawnSemaphore->Acquire();

	// Get the first item from the map // TODO Might be worth replacing the map with a Queue
	if (!flowersSpawnPositions.IsEmpty()) {
		for (const TPair<FIntPoint, TArray<FVoxelObjectLocationData>>& pair : flowersSpawnPositions) {
			output = pair.Value;
			flowersSpawnPositions.Remove(pair.Key);
			break;
		}
	}

	FlowersToSpawnSemaphore->Release();
	return output;
}

TArray<FVoxelObjectLocationData> UChunkLocationData::getNPCSpawnPosition() {
	TArray<FVoxelObjectLocationData> output;

	NPCToSpawnSemaphore->Acquire();
	
	// Get the first item from the map // TODO Maybe extractt this into a function, as all getSpawnPosition use the same logic
	if (!NPCSpawnPositions.IsEmpty()) {
		for (const TPair<FIntPoint, TArray<FVoxelObjectLocationData>>& pair : NPCSpawnPositions) {
			output = pair.Value;
			NPCSpawnPositions.Remove(pair.Key);
			break;
		}
	}

	NPCToSpawnSemaphore->Release();
	return output;
}

void UChunkLocationData::addTreeSpawnPosition(const FVoxelObjectLocationData position) {
	TreesToSpawnSemaphore->Acquire();

	// If it exists, add the new tree position to the existing array
	if (treesSpawnPositions.Contains(position.ObjectWorldCoords)) {
		treesSpawnPositions[position.ObjectWorldCoords].Add(position);
	} else {
		// If not, create a new array with the new tree position
		treesSpawnPositions.Add(position.ObjectWorldCoords, TArray<FVoxelObjectLocationData>({ position }));
	}

	TreesToSpawnSemaphore->Release();
}

void UChunkLocationData::addGrassSpawnPosition(const FVoxelObjectLocationData position) {
	GrassToSpawnSemaphore->Acquire();

	// If it exists, add the new grass position to the existing array
	if (grassSpawnPositions.Contains(position.ObjectWorldCoords)) {
		grassSpawnPositions[position.ObjectWorldCoords].Add(position);
	} else {
		// If not, create a new array with the new grass position
		grassSpawnPositions.Add(position.ObjectWorldCoords, TArray<FVoxelObjectLocationData>({ position }));
	}

	GrassToSpawnSemaphore->Release();
}

void UChunkLocationData::addFlowerSpawnPosition(const FVoxelObjectLocationData position) {
	FlowersToSpawnSemaphore->Acquire();

	// If it exists, add the new flower position to the existing array
	if (flowersSpawnPositions.Contains(position.ObjectWorldCoords)) {
		flowersSpawnPositions[position.ObjectWorldCoords].Add(position);
	} else {
		// If not, create a new array with the new flower position
		flowersSpawnPositions.Add(position.ObjectWorldCoords, TArray<FVoxelObjectLocationData>({ position }));
	}

	FlowersToSpawnSemaphore->Release();
}

void UChunkLocationData::addNPCSpawnPosition(const FVoxelObjectLocationData position) {
	NPCToSpawnSemaphore->Acquire();

	// If it exists, add the new NPC position to the existing array
	if (NPCSpawnPositions.Contains(position.ObjectWorldCoords)) {
		NPCSpawnPositions[position.ObjectWorldCoords].Add(position);
	} else {
		// If not, create a new array with the new NPC position
		NPCSpawnPositions.Add(position.ObjectWorldCoords, TArray<FVoxelObjectLocationData>({ position }));
	}

	NPCToSpawnSemaphore->Release();
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

void UChunkLocationData::RemoveGrassSpawnPosition(const FIntPoint& point) {
	GrassToSpawnSemaphore->Acquire();

	// Iterating over the map and removing the key and value
	for (TMap<FIntPoint, TArray<FVoxelObjectLocationData>>::TIterator SpawnPosition(grassSpawnPositions); SpawnPosition; ++SpawnPosition) {
		if (SpawnPosition.Key() == point) {
			SpawnPosition.RemoveCurrent();
			break;
		}
	}

	GrassToSpawnSemaphore->Release();
}

void UChunkLocationData::RemoveFlowerSpawnPosition(const FIntPoint& point) {
	FlowersToSpawnSemaphore->Acquire();

	// Iterating over the map and removing the key and value
	for (TMap<FIntPoint, TArray<FVoxelObjectLocationData>>::TIterator SpawnPosition(flowersSpawnPositions); SpawnPosition; ++SpawnPosition) {
		if (SpawnPosition.Key() == point) {
			SpawnPosition.RemoveCurrent();
			break;
		}
	}

	FlowersToSpawnSemaphore->Release();
}

void UChunkLocationData::RemoveNPCSpawnPosition(const FIntPoint& point) {
	NPCToSpawnSemaphore->Acquire();

	// Iterating over the map and removing the key and value
	for (TMap<FIntPoint, TArray<FVoxelObjectLocationData>>::TIterator SpawnPosition(NPCSpawnPositions); SpawnPosition; ++SpawnPosition) {
		if (SpawnPosition.Key() == point) {
			SpawnPosition.RemoveCurrent();
			break;
		}
	}

	NPCToSpawnSemaphore->Release();
}

bool UChunkLocationData::AddUnspawnedTreeToDestroy(ATree* InTreeToDestroy) {
	return unspawnedTreesToDestroy.Enqueue(InTreeToDestroy);
}

bool UChunkLocationData::GetUnspawnedTreeToDestroy(ATree* InTreeToDestroy) {
	return unspawnedTreesToDestroy.Dequeue(InTreeToDestroy);
}

bool UChunkLocationData::AddUnspawnedGrassToDestroy(UProceduralMeshComponent* InGrassToDestroy) {
	return unspawnedGrassToDestroy.Enqueue(InGrassToDestroy);
}

bool UChunkLocationData::GetUnspawnedGrassToDestroy(UProceduralMeshComponent* InGrassToDestroy) {
	return unspawnedGrassToDestroy.Dequeue(InGrassToDestroy);
}

bool UChunkLocationData::AddUnspawnedFlowerToDestroy(UProceduralMeshComponent* InFlowerToDestroy) {
	return unspawnedFlowerToDestroy.Enqueue(InFlowerToDestroy);
}

bool UChunkLocationData::GetUnspawnedFlowerToDestroy(UProceduralMeshComponent* InFlowerToDestroy) {
	return unspawnedFlowerToDestroy.Dequeue(InFlowerToDestroy);
}

void UChunkLocationData::AddSurfaceVoxelPointsForChunk(const FIntPoint& chunkPosition, const TArray<int>& voxelPoints, const TArray<FVector2D>& avoidPoints) {
	SurfaceVoxelPointsSemaphore->Acquire();
	surfaceVoxelPoints.Add(chunkPosition, voxelPoints);
	surfaceAvoidPoints.Add(chunkPosition, avoidPoints);
	SurfaceVoxelPointsSemaphore->Release();
}

void UChunkLocationData::RemoveSurfaceVoxelPointsForChunk(const FIntPoint& chunkPosition) {
	SurfaceVoxelPointsSemaphore->Acquire();
	surfaceVoxelPoints.Remove(chunkPosition);
	surfaceAvoidPoints.Remove(chunkPosition);
	SurfaceVoxelPointsSemaphore->Release();
}

TMap<FIntPoint, TArray<int>> UChunkLocationData::GetSurfaceVoxelPoints() {
    TMap<FIntPoint, TArray<int>> result;
    SurfaceVoxelPointsSemaphore->Acquire();
    result = surfaceVoxelPoints;
    SurfaceVoxelPointsSemaphore->Release();
    return result;
}

bool UChunkLocationData::IsSurfacePointValid(const double& X, const double& Z) {
	// Adjust the coordinates to they're relative to the coordinates inside a single chunk
	const int relativeToChunkX = FMath::Floor(X / chunkSize);
	const int relativeToChunkZ = FMath::Floor(Z / chunkSize);

    const FIntPoint chunkCoords = FIntPoint(relativeToChunkX, relativeToChunkZ);

    TArray<FVector2D> avoidPoints;
    SurfaceVoxelPointsSemaphore->Acquire();
    if (surfaceAvoidPoints.Contains(chunkCoords)) {
        avoidPoints = surfaceAvoidPoints[chunkCoords];
    }
    SurfaceVoxelPointsSemaphore->Release();

    // Check if any point in avoidPoints contains the points given
    const double modX = FMath::Max(((static_cast<int>(X) % chunkSize) + chunkSize) % chunkSize - 1, 0);
    const double modZ = FMath::Max(((static_cast<int>(Z) % chunkSize) + chunkSize) % chunkSize - 1, 0);

    for (const FVector2D& point : avoidPoints) {
        if (point.X == modX && point.Y == modZ) {
            return false;
        }
    }

    return true;
}
