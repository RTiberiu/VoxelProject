#include "ChunkLocationData.h"

#include "..\TerrainSettings\WorldTerrainSettings.h"

UChunkLocationData::UChunkLocationData() :
	ChunksToSpawnSemaphore(new FairSemaphore(1)),
	ChunksToDestroySemaphore(new FairSemaphore(1)),
	MeshDataSemaphore(new FairSemaphore(1)),
	GrassToSpawnSemaphore(new FairSemaphore(1)),
	FlowersToSpawnSemaphore(new FairSemaphore(1)),
	NPCToSpawnSemaphore(new FairSemaphore(1)),
	TreesToSpawnSemaphore(new FairSemaphore(1)),
	VegetationChunkSemaphore(new FairSemaphore(1)),
	TreeChunkSemaphore(new FairSemaphore(1)),
	NpcChunkSemaphore(new FairSemaphore(1)),
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
	delete VegetationChunkSemaphore;
	delete TreeChunkSemaphore;
	delete NpcChunkSemaphore;
}

void UChunkLocationData::SetWorldTerrainSettings(UWorldTerrainSettings* InWorldTerrainSettings) {
	WorldTerrainSettingsRef = InWorldTerrainSettings;
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

void UChunkLocationData::AddVegetationChunkSpawnPosition(FIntPoint& chunkPosition) {
	VegetationChunkSemaphore->Acquire();
	if (!VegetationChunkSpawnPoints.Contains(chunkPosition)) {  
		VegetationChunkSpawnPoints.Add(chunkPosition, nullptr);  
	}  
	VegetationChunkSemaphore->Release();
}

void UChunkLocationData::AddTreeChunkSpawnPosition(FIntPoint& chunkPosition) {
	TreeChunkSemaphore->Acquire();
	if (!TreeChunkSpawnPoints.Contains(chunkPosition)) {
		TreeChunkSpawnPoints.Add(chunkPosition, nullptr);
	}
	TreeChunkSemaphore->Release();
}

void UChunkLocationData::AddNpcChunkSpawnPosition(FIntPoint& chunkPosition) {
	NpcChunkSemaphore->Acquire();
	if (!NpcChunkSpawnPoints.Contains(chunkPosition)) {
		NpcChunkSpawnPoints.Add(chunkPosition, nullptr);
	}
	NpcChunkSemaphore->Release();
}

void UChunkLocationData::RemoveVegetationChunkSpawnPosition(FIntPoint& chunkPosition) {
	VegetationChunkSemaphore->Acquire();
	if (VegetationChunkSpawnPoints.Contains(chunkPosition)) {
		VegetationChunkSpawnPoints.Remove(chunkPosition);
	}

	// Remove grass, flowers, and trees waiting to be spawned
	if (grassInRangeSpawnPositions.Contains(chunkPosition)) {
		grassInRangeSpawnPositions.Remove(chunkPosition);
	}

	if (flowersInRangeSpawnPositions.Contains(chunkPosition)) {
		flowersInRangeSpawnPositions.Remove(chunkPosition);
	}

	VegetationChunkSemaphore->Release();
}

void UChunkLocationData::RemoveTreeChunkSpawnPosition(FIntPoint& chunkPosition) {
	TreeChunkSemaphore->Acquire();
	if (TreeChunkSpawnPoints.Contains(chunkPosition)) {
		TreeChunkSpawnPoints.Remove(chunkPosition);
	}

	if (treesInRangeSpawnPositions.Contains(chunkPosition)) {
		treesInRangeSpawnPositions.Remove(chunkPosition);
	}

	TreeChunkSemaphore->Release();
}

void UChunkLocationData::RemoveNpcChunkSpawnPosition(FIntPoint& chunkPosition) {
	NpcChunkSemaphore->Acquire();
	if (NpcChunkSpawnPoints.Contains(chunkPosition)) {
		NpcChunkSpawnPoints.Remove(chunkPosition);
	}

	if (npcInRangeSpawnPositions.Contains(chunkPosition)) {
		npcInRangeSpawnPositions.Remove(chunkPosition);
	}
	NpcChunkSemaphore->Release();
}

void UChunkLocationData::CheckForSpawnPointsInRange() {
	VegetationChunkSemaphore->Acquire();  
	
	// Check if Vegetation spawn points are in Grass spawn points, and add a reference
	// to that list of Grass points if it exists.
	for (const TPair<FIntPoint, TArray<FVoxelObjectLocationData>*>& VegetationPair : VegetationChunkSpawnPoints) {  
		// Adding grass spawn points
		GrassToSpawnSemaphore->Acquire();
		if (grassSpawnPositions.Contains(VegetationPair.Key)) {
			if (grassInRangeSpawnPositions.Find(VegetationPair.Key) == nullptr) {
				grassInRangeSpawnPositions.Add(
					VegetationPair.Key,
					&grassSpawnPositions[VegetationPair.Key]
				);
			}
		}
		GrassToSpawnSemaphore->Release();

		// Adding flower spawn points
		FlowersToSpawnSemaphore->Acquire();
		if (flowersSpawnPositions.Contains(VegetationPair.Key)) {
			if (flowersInRangeSpawnPositions.Find(VegetationPair.Key) == nullptr) {
				flowersInRangeSpawnPositions.Add(
					VegetationPair.Key,
					&flowersSpawnPositions[VegetationPair.Key]
				);
			}
		}
		FlowersToSpawnSemaphore->Release();

		// Adding trees spawn points // COMMENTED OUT TO KEEP THEM OUT OF THE LOD SYSTEM
		if (flowersSpawnPositions.Contains(VegetationPair.Key)) {
			if (flowersInRangeSpawnPositions.Find(VegetationPair.Key) == nullptr) {
				flowersInRangeSpawnPositions.Add(
					VegetationPair.Key,
					&flowersSpawnPositions[VegetationPair.Key]
				);
			}
		}
	}

	VegetationChunkSemaphore->Release();  

	// Checking the tree spawn area
	TreeChunkSemaphore->Acquire();
	TreesToSpawnSemaphore->Acquire();
	for (const TPair<FIntPoint, TArray<FVoxelObjectLocationData>*>& TreePair : TreeChunkSpawnPoints) {
		if (treesSpawnPositions.Contains(TreePair.Key)) {
			if (treesInRangeSpawnPositions.Find(TreePair.Key) == nullptr) {
				treesInRangeSpawnPositions.Add(
					TreePair.Key,
					&treesSpawnPositions[TreePair.Key]
				);
			}
		}
	}
	TreesToSpawnSemaphore->Release();
	TreeChunkSemaphore->Release();

	// Checking the NPC spawn area
	NpcChunkSemaphore->Acquire();
	NPCToSpawnSemaphore->Acquire();
	for (const TPair<FIntPoint, TArray<TPair<FVoxelObjectLocationData, AnimalType>>*>& NpcPair : NpcChunkSpawnPoints) {
		if (npcSpawnPositions.Contains(NpcPair.Key)) {
			if (npcInRangeSpawnPositions.Find(NpcPair.Key) == nullptr) {
				npcInRangeSpawnPositions.Add(
					NpcPair.Key,
					&npcSpawnPositions[NpcPair.Key]
				);
			}
		}
	}
	NPCToSpawnSemaphore->Release();
	NpcChunkSemaphore->Release();
}

void UChunkLocationData::CheckAndAddVegetationNotInRange(  
	TQueue<UCustomProceduralMeshComponent*>* GrassActorsToRemove,  
	TQueue<UCustomProceduralMeshComponent*>* FlowerActorsToRemove) {
   VegetationChunkSemaphore->Acquire();  

   // Get the Chunk Coordinates from Vegetation Chunk Spawn Points
   TArray<FIntPoint> Keys;  
   VegetationChunkSpawnPoints.GetKeys(Keys);  
	
   WTSR->CheckAndReturnGrassNotInRange(Keys, GrassActorsToRemove);
   WTSR->CheckAndReturnFlowersNotInRange(Keys, FlowerActorsToRemove);

   VegetationChunkSemaphore->Release();  
}

void UChunkLocationData::CheckAndAddTreesNotInRange(TQueue<ATree*>* TreeActorsToRemove) {
	TreeChunkSemaphore->Acquire();

	// Get the Chunk Coordinates from Tree Chunk Spawn Points
	TArray<FIntPoint> Keys;
	TreeChunkSpawnPoints.GetKeys(Keys);

	WTSR->CheckAndReturnTreesNotInRange(Keys, TreeActorsToRemove);

	TreeChunkSemaphore->Release();
}

void UChunkLocationData::CheckAndAddNpcsNotInRange(TQueue<ABasicNPC*>* NpcActorsToRemove) {
	NpcChunkSemaphore->Acquire();

	// Get the Chunk Coordinates from Tree Chunk Spawn Points
	TArray<FIntPoint> Keys;
	NpcChunkSpawnPoints.GetKeys(Keys);

	WTSR->CheckAndReturnNpcsNotInRange(Keys, NpcActorsToRemove);

	NpcChunkSemaphore->Release();	
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

TArray<TPair<FVoxelObjectLocationData, AnimalType>> UChunkLocationData::getNPCSpawnPosition() {
	TArray<TPair<FVoxelObjectLocationData, AnimalType>> output;

	NPCToSpawnSemaphore->Acquire();
	
	// Get the first item from the map // TODO Maybe extractt this into a function, as all getSpawnPosition use the same logic
	if (!npcSpawnPositions.IsEmpty()) {
		for (const TPair<FIntPoint, TArray<TPair<FVoxelObjectLocationData, AnimalType>>>& pair : npcSpawnPositions) {
			output = pair.Value;
			npcSpawnPositions.Remove(pair.Key);
			break;
		}
	}

	NPCToSpawnSemaphore->Release();
	return output;
}

TArray<FVoxelObjectLocationData> UChunkLocationData::getTreeSpawnPositionsInRange() {
	TArray<FVoxelObjectLocationData> output;
	VegetationChunkSemaphore->Acquire();

	for (TPair<FIntPoint, TArray<FVoxelObjectLocationData>*>& pair : treesInRangeSpawnPositions) {
		if (TArray<FVoxelObjectLocationData>* FoundPtr = treesSpawnPositions.Find(pair.Key)) {
			TArray<FVoxelObjectLocationData>* OriginalPtr = FoundPtr;
			if (pair.Value == OriginalPtr && pair.Value->Num() > 0) {
				output = *pair.Value;
				pair.Value->Empty();
				break;
			}
		}
	}

	VegetationChunkSemaphore->Release();
	return output;
}

TArray<FVoxelObjectLocationData> UChunkLocationData::getGrassSpawnPositionInRange() {
	TArray<FVoxelObjectLocationData> output;  
	VegetationChunkSemaphore->Acquire();

	for (TPair<FIntPoint, TArray<FVoxelObjectLocationData>*>& pair : grassInRangeSpawnPositions) {  
		if (TArray<FVoxelObjectLocationData>* FoundPtr = grassSpawnPositions.Find(pair.Key)) {
			TArray<FVoxelObjectLocationData>* OriginalPtr = FoundPtr;
			if (pair.Value == OriginalPtr && pair.Value->Num() > 0) {
				output = *pair.Value;
				pair.Value->Empty();
				break;
			}
		}
	}  

	VegetationChunkSemaphore->Release();
	return output;  
}

TArray<FVoxelObjectLocationData> UChunkLocationData::getFlowerSpawnPositionInRange() {
	TArray<FVoxelObjectLocationData> output;
	VegetationChunkSemaphore->Acquire();

	for (TPair<FIntPoint, TArray<FVoxelObjectLocationData>*>& pair : flowersInRangeSpawnPositions) {
		if (TArray<FVoxelObjectLocationData>* FoundPtr = flowersSpawnPositions.Find(pair.Key)) {
			TArray<FVoxelObjectLocationData>* OriginalPtr = FoundPtr;
			if (pair.Value == OriginalPtr && pair.Value->Num() > 0) {
				output = *pair.Value;
				pair.Value->Empty();
				break;
			}
		}
	}

	VegetationChunkSemaphore->Release();
	return output;
}

TArray<TPair<FVoxelObjectLocationData, AnimalType>> UChunkLocationData::getNPCSpawnPositionInRange() {
	TArray<TPair<FVoxelObjectLocationData, AnimalType>> output;
	NpcChunkSemaphore->Acquire();

	for (TPair<FIntPoint, TArray<TPair<FVoxelObjectLocationData, AnimalType>>*>& pair : npcInRangeSpawnPositions) {
		if (TArray<TPair<FVoxelObjectLocationData, AnimalType>>* FoundPtr = npcSpawnPositions.Find(pair.Key)) {
			TArray<TPair<FVoxelObjectLocationData, AnimalType>>* OriginalPtr = FoundPtr;
			if (pair.Value == OriginalPtr && pair.Value->Num() > 0) {
				output = *pair.Value;
				pair.Value->Empty();
				break;
			}
		}
	}

	NpcChunkSemaphore->Release();
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

void UChunkLocationData::addNPCSpawnPosition(const TPair<FVoxelObjectLocationData, AnimalType> positionAndType) {
	NPCToSpawnSemaphore->Acquire();
	
	// If it exists, add the new NPC position to the existing array
	if (npcSpawnPositions.Contains(positionAndType.Key.ObjectWorldCoords)) {
		npcSpawnPositions[positionAndType.Key.ObjectWorldCoords].Add(positionAndType);
	} else {
		// If not, create a new array with the new NPC position
		npcSpawnPositions.Add(positionAndType.Key.ObjectWorldCoords, TArray<TPair<FVoxelObjectLocationData, AnimalType>>({ positionAndType }));
	}

	NPCToSpawnSemaphore->Release();
}

void UChunkLocationData::addTreeSpawnPositions(const TArray<FVoxelObjectLocationData>& positions) {
    TreesToSpawnSemaphore->Acquire();
    for (const FVoxelObjectLocationData& pos : positions) {
        if (treesSpawnPositions.Contains(pos.ObjectWorldCoords)) {
            treesSpawnPositions[pos.ObjectWorldCoords].Add(pos);
        } else {
            treesSpawnPositions.Add(pos.ObjectWorldCoords, TArray<FVoxelObjectLocationData>({ pos }));
        }
    }
    TreesToSpawnSemaphore->Release();
}

void UChunkLocationData::addGrassSpawnPositions(const TArray<FVoxelObjectLocationData>& positions) {
    GrassToSpawnSemaphore->Acquire();
    for (const FVoxelObjectLocationData& pos : positions) {
        if (grassSpawnPositions.Contains(pos.ObjectWorldCoords)) {
            grassSpawnPositions[pos.ObjectWorldCoords].Add(pos);
        } else {
            grassSpawnPositions.Add(pos.ObjectWorldCoords, TArray<FVoxelObjectLocationData>({ pos }));
        }
    }
    GrassToSpawnSemaphore->Release();
}

void UChunkLocationData::addFlowerSpawnPositions(const TArray<FVoxelObjectLocationData>& positions) {
    FlowersToSpawnSemaphore->Acquire();
    for (const FVoxelObjectLocationData& pos : positions) {
        if (flowersSpawnPositions.Contains(pos.ObjectWorldCoords)) {
            flowersSpawnPositions[pos.ObjectWorldCoords].Add(pos);
        } else {
            flowersSpawnPositions.Add(pos.ObjectWorldCoords, TArray<FVoxelObjectLocationData>({ pos }));
        }
    }
    FlowersToSpawnSemaphore->Release();
}

void UChunkLocationData::addNPCSpawnPositions(const TArray<TPair<FVoxelObjectLocationData, AnimalType>>& positionsAndTypes) {
    NPCToSpawnSemaphore->Acquire();
    for (const TPair<FVoxelObjectLocationData, AnimalType>& entry : positionsAndTypes) {
        const FIntPoint& key = entry.Key.ObjectWorldCoords;
        if (npcSpawnPositions.Contains(key)) {
            npcSpawnPositions[key].Add(entry);
        } else {
            npcSpawnPositions.Add(key, TArray<TPair<FVoxelObjectLocationData, AnimalType>>({ entry }));
        }
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
	for (TMap<FIntPoint, TArray<TPair<FVoxelObjectLocationData, AnimalType>>>::TIterator SpawnPosition(npcSpawnPositions); SpawnPosition; ++SpawnPosition) {
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

bool UChunkLocationData::AddUnspawnedNpcToDestroy(ABasicNPC* InNpcToDestroy) {
	return unspawnedNpcsToDestroy.Enqueue(InNpcToDestroy);
}

bool UChunkLocationData::GetUnspawnedNpcToDestroy(ABasicNPC* InNpcToDestroy) {
	return unspawnedNpcsToDestroy.Dequeue(InNpcToDestroy);
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

	bool isAvoidedPoint = avoidPoints.Contains(FVector2D(modX, modZ));
	if (isAvoidedPoint) {
		return false;
	}

	// Check if any point is occupied by a current NPC 
	FIntPoint point = FIntPoint(X * WTSR->UnrealScale + WTSR->HalfUnrealScale, Z * WTSR->UnrealScale + WTSR->HalfUnrealScale); // Adjusting 2D points to Unreal's scale
    return !IsLocationOccupied(point);
}

bool UChunkLocationData::IsLocationOccupied(const FVector& currentPosition, const FVector& nextPosition, ABasicNPC* npcAtLocation) {
	FIntPoint nextPositionPoint = FIntPoint(nextPosition.X, nextPosition.Y); 

	if (OccupiedVoxels.Contains(nextPositionPoint)) {
		return true;
	} 

	// Remove the previous NPC location to allow other NPCs to move to that position
	FIntPoint previousPoint = FIntPoint(currentPosition.X, currentPosition.Y);
	if (OccupiedVoxels.Contains(previousPoint)) {
		OccupiedVoxels.Remove(previousPoint);
	}

	// Add the new position as occupied
	OccupiedVoxels.Add(nextPositionPoint, npcAtLocation);

	return false;
}

bool UChunkLocationData::IsLocationOccupied(const FIntPoint& position) {
	return OccupiedVoxels.Contains(position);
}

void UChunkLocationData::RemoveOccupiedVoxelPosition(const FVector& position) {  
   FIntPoint point = FIntPoint(position.X, position.Y);  

   // Remove the position from the OccupiedVoxels map if it exists  
   if (OccupiedVoxels.Contains(point)) {  
       OccupiedVoxels.Remove(point);  
   }  
}

void UChunkLocationData::AddOccupiedVoxelPosition(const FVector& position, ABasicNPC* npcAtLocation) {
	FIntPoint point = FIntPoint(position.X, position.Y);
	OccupiedVoxels.Add(point, npcAtLocation);
}

TMap<FIntPoint, ABasicNPC*> UChunkLocationData::GetOccupiedVoxels() const {
	return OccupiedVoxels;
}
