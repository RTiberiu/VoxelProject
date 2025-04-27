// Fill out your copyright notice in the Description page of Project Settings.

#include "WorldTerrainSettings.h"
#include "..\..\Noise\PerlinNoiseSettings.h"
#include "..\..\Noise\NoiseLibrary\FastNoiseLite.h"
#include "..\..\Utils\Semaphore\FairSemaphore.h"


UWorldTerrainSettings::UWorldTerrainSettings() :
	UpdateChunkSemaphore(new FairSemaphore(1)),
	PlayerPositionSemaphore(new FairSemaphore(1)),
	ChunkMapSemaphore(new FairSemaphore(1)),
	TreeMapSemaphore(new FairSemaphore(1)),
	GrassMapSemaphore(new FairSemaphore(1)),
	FlowerMapSemaphore(new FairSemaphore(1)),
	NpcMapSemaphore(new FairSemaphore(1)),
	DrawDistanceSemaphore(new FairSemaphore(1)),
	AddCollisionTreesSemaphore(new FairSemaphore(1)),
	RemoveCollisionTreesSemaphore(new FairSemaphore(1)),
	AddCollisionChunksSemaphore(new FairSemaphore(1)),
	RemoveCollisionChunksSemaphore(new FairSemaphore(1)) {
	// Reserve memory for double the draw distance for X and Z 
	SpawnedChunksMap.Reserve(DrawDistance * DrawDistance * 2);
}

UWorldTerrainSettings::~UWorldTerrainSettings() {
	// Clean up semaphores
	delete UpdateChunkSemaphore;
	delete PlayerPositionSemaphore;
	delete ChunkMapSemaphore;
	delete TreeMapSemaphore;
	delete GrassMapSemaphore;
	delete FlowerMapSemaphore;
	delete NpcMapSemaphore;
	delete DrawDistanceSemaphore;

	delete AddCollisionTreesSemaphore;
	delete RemoveCollisionTreesSemaphore;
	delete AddCollisionChunksSemaphore;
	delete RemoveCollisionChunksSemaphore;

	if (PerlinNoiseSettingsRef) {
		PerlinNoiseSettingsRef = nullptr;
	}
}

void UWorldTerrainSettings::AddChunkToMap(const FIntPoint& ChunkCoordinates, AActor* ChunkActor) {
	ChunkMapSemaphore->Acquire();
	ValidateSpawnedChunksMap(); // TESTING
	SpawnedChunksMap.Add(ChunkCoordinates, ChunkActor);
	ValidateSpawnedChunksMap(); // TESTING
	ChunkMapSemaphore->Release();
}

AActor* UWorldTerrainSettings::GetAndRemoveChunkFromMap(const FIntPoint& ChunkCoordinates) {
	ChunkMapSemaphore->Acquire();
	ValidateSpawnedChunksMap(); // TESTING
	AActor* RemovedChunk = nullptr;
	if (SpawnedChunksMap.Contains(ChunkCoordinates)) {
		RemovedChunk = SpawnedChunksMap.FindAndRemoveChecked(ChunkCoordinates);
	}
	ValidateSpawnedChunksMap(); // TESTING
	ChunkMapSemaphore->Release();
	return RemovedChunk;
}

AActor* UWorldTerrainSettings::GetNextChunkFromMap() {
	ChunkMapSemaphore->Acquire();
	AActor* RemovedChunk = nullptr;
	FIntPoint keyToRemove = FIntPoint::ZeroValue;

	if (SpawnedChunksMap.Num() > 0) {
		// Get the first item in the map
		for (TPair<FIntPoint, AActor*>& Elem : SpawnedChunksMap) {
			RemovedChunk = Elem.Value;
			keyToRemove = Elem.Key;
			break;
		}
		// Remove the retrived chunk from the map 
		SpawnedChunksMap.Remove(keyToRemove);
	}
	ChunkMapSemaphore->Release();
	return RemovedChunk;
}

int UWorldTerrainSettings::GetMapSize() {
	ChunkMapSemaphore->Acquire();
	int MapSize = SpawnedChunksMap.Num();
	ChunkMapSemaphore->Release();
	return MapSize;
}

void UWorldTerrainSettings::updateInitialPlayerPosition(FVector newPosition) {
	PlayerPositionSemaphore->Acquire();
	playerInitialPosition = newPosition;
	PlayerPositionSemaphore->Release();
}

FVector UWorldTerrainSettings::getInitialPlayerPosition() {
	PlayerPositionSemaphore->Acquire();
	FVector Position = playerInitialPosition;
	PlayerPositionSemaphore->Release();
	return Position;
}

void UWorldTerrainSettings::updateCurrentPlayerPosition(FVector& newPosition) {
	playerCurrentPosition = newPosition;
}

FVector& UWorldTerrainSettings::getCurrentPlayerPosition() {
	return playerCurrentPosition;
}

void UWorldTerrainSettings::EmptyChunkMap() {
	ChunkMapSemaphore->Acquire();
	SpawnedChunksMap.Empty();
	ChunkMapSemaphore->Release();
}

// Get a read-only version of the map
const TMap<FIntPoint, AActor*>& UWorldTerrainSettings::GetSpawnedChunksMap() const {
	return SpawnedChunksMap;
}

/*
* Get read-only items of SpawnedChunksMap and iterate to see if chunks are outside or inside of
* the collision threshold. Chunks inside the threshold will have their meshes regenerated with
* collision and chunks outside of it will get their collision disabled through a mesh update.
*/
void UWorldTerrainSettings::UpdateChunksCollision(FVector& PlayerPosition) {
	ChunkMapSemaphore->Acquire();

	for (const TPair<FIntPoint, AActor*>& ChunkPair : SpawnedChunksMap) {
		ABinaryChunk* ChunkActor = Cast<ABinaryChunk>(ChunkPair.Value);

		if (ChunkActor) {
			const FVector ChunkPosition = ChunkActor->GetActorLocation();

			// Define the boundaries for the collision check
			float minX = PlayerPosition.X - CollisionDistance;
			float maxX = PlayerPosition.X + CollisionDistance;
			float minY = PlayerPosition.Y - CollisionDistance;
			float maxY = PlayerPosition.Y + CollisionDistance;

			// Check if the player is within the collision boundaries
			bool withinCollisionDistance = (ChunkPosition.X >= minX && ChunkPosition.X <= maxX) &&
				(ChunkPosition.Y >= minY && ChunkPosition.Y <= maxY);

			// Update collision state based on proximity
			if (withinCollisionDistance) {
				if (!ChunkActor->HasCollision()) {
					AddCollisionChunksSemaphore->Acquire();
					AddCollisionChunks.Add(ChunkActor);
					AddCollisionChunksSemaphore->Release();

				}
			}
			else {
				if (ChunkActor->HasCollision()) {
					RemoveCollisionChunksSemaphore->Acquire();
					RemoveCollisionChunks.Add(ChunkActor);
					RemoveCollisionChunksSemaphore->Release();
				}
			}
		}
	}

	ChunkMapSemaphore->Release();
}

void UWorldTerrainSettings::UpdateTreeCollisions(FVector& PlayerPosition) {
	TreeMapSemaphore->Acquire();

	const TMap<FIntPoint, TArray<ATree*>> SpawnedTreesMapTemp = SpawnedTreesMap;

	TreeMapSemaphore->Release(); // TODO THIS MIGHT BREAK, SINCE THE TREE ACTOR MIGHT NOT EXIST WHEN I TRY TO UPDATE IT

	for (const TPair<FIntPoint, TArray<ATree*>>& treesAtLocation : SpawnedTreesMapTemp) {
		// Iterate through each tree in the array for this specific location
		for (ATree* tree : treesAtLocation.Value) {
			if (tree) {
				const FVector ChunkPosition = tree->GetActorLocation();

				// Define the boundaries for the collision check
				float minX = PlayerPosition.X - VegetationCollisionDistance;
				float maxX = PlayerPosition.X + VegetationCollisionDistance;
				float minY = PlayerPosition.Y - VegetationCollisionDistance;
				float maxY = PlayerPosition.Y + VegetationCollisionDistance;

				// Check if the player is within the collision boundaries
				bool withinCollisionDistance = (ChunkPosition.X >= minX && ChunkPosition.X <= maxX) &&
					(ChunkPosition.Y >= minY && ChunkPosition.Y <= maxY);

				// Update collision state based on proximity
				if (withinCollisionDistance) {
					if (!tree->HasCollision()) {
						AddCollisionTreesSemaphore->Acquire();
						AddCollisionTrees.Add(tree);
						AddCollisionTreesSemaphore->Release();
					}
				}
				else {
					if (tree->HasCollision()) {
						RemoveCollisionTreesSemaphore->Acquire();
						RemoveCollisionTrees.Add(tree);
						RemoveCollisionTreesSemaphore->Release();
					}
				}
			}
		}
	}

}

bool UWorldTerrainSettings::isActorPresentInMap(AActor* actor) {
	for (const TPair<FIntPoint, AActor*>& Pair : SpawnedChunksMap) {
		if (Pair.Value == actor) {
			return true;
		}
	}
	return false;
}

void UWorldTerrainSettings::printMapElements(FString message) {
	int testCounter{ 0 };
	FString keysString = message + " Map count: " + FString::FromInt(SpawnedChunksMap.Num()) + ": \n";

	for (const TPair<FIntPoint, AActor*>& Pair : SpawnedChunksMap) {

		if (!Pair.Value) {
			UE_LOG(LogTemp, Error, TEXT("Null actor found in SpawnedChunksMap at coordinates X:%d, Z:%d"), Pair.Key.X, Pair.Key.Y);
			continue;
		}

		keysString += FString::Printf(TEXT("\tX:%d, Z:%d "), Pair.Key.X, Pair.Key.Y);
		testCounter++;

		if (testCounter == DrawDistance * 2) {
			keysString += TEXT("\n");
			testCounter = 0;
		}
	}
	UE_LOG(LogTemp, Warning, TEXT("%s"), *keysString);
}

void UWorldTerrainSettings::ValidateSpawnedChunksMap() {
	CheckForDuplicateActorPointers();

	CheckIfActorIsNullOrPendingKill();

	CheckForDuplicateWorldCoordinates();

	// CheckNumberOfElements();
}

void UWorldTerrainSettings::AddTreeMeshData(FVoxelObjectMeshData treeData) {
	TreesMeshData.Add(treeData);
}

void UWorldTerrainSettings::AddGrassMeshData(FVoxelObjectMeshData grassData) {
	GrassMeshData.Add(grassData);
}

void UWorldTerrainSettings::AddFlowerMeshData(FVoxelObjectMeshData flowerData) {
	FlowersMeshData.Add(flowerData);
}

FVoxelObjectMeshData* UWorldTerrainSettings::GetRandomTreeMeshData() {
	return &TreesMeshData[FMath::RandRange(0, TreesMeshData.Num() - 1)];
}

FVoxelObjectMeshData* UWorldTerrainSettings::GetRandomGrassMeshData() {
	return &GrassMeshData[FMath::RandRange(0, GrassMeshData.Num() - 1)];

}

FVoxelObjectMeshData* UWorldTerrainSettings::GetRandomFlowerMeshData() {
	return &FlowersMeshData[FMath::RandRange(0, FlowersMeshData.Num() - 1)];
}

void UWorldTerrainSettings::AddSpawnedTrees(const FIntPoint& TreeWorldCoordinates, ATree* TreeActor) {
	TreeMapSemaphore->Acquire();

	// If it exists, add the new tree to the existing array
	if (SpawnedTreesMap.Contains(TreeWorldCoordinates)) {
		SpawnedTreesMap[TreeWorldCoordinates].Add(TreeActor);
	} else {
		// If not, create a new array with the new tree
		SpawnedTreesMap.Add(TreeWorldCoordinates, TArray<ATree*>({ TreeActor }));
	}

	TreeMapSemaphore->Release();
}

void UWorldTerrainSettings::AddSpawnedGrass(const FIntPoint& GrassWorldCoordinates, UCustomProceduralMeshComponent* GrassActor) {
	GrassMapSemaphore->Acquire();

	// If it exists, add the new grass to the existing array
	if (SpawnedGrassMap.Contains(GrassWorldCoordinates)) {
		SpawnedGrassMap[GrassWorldCoordinates].Add(GrassActor);
	} else {
		// If not, create a new array with the new grass
		SpawnedGrassMap.Add(GrassWorldCoordinates, TArray<UCustomProceduralMeshComponent*>({ GrassActor }));
	}

	GrassMapSemaphore->Release();
}

void UWorldTerrainSettings::AddSpawnedFlower(const FIntPoint& FlowerWorldCoordinates, UCustomProceduralMeshComponent* FlowerActor) {
	FlowerMapSemaphore->Acquire();

	// If the coordinate of the chunk exists, add the new flower to the existing array
	if (SpawnedFlowerMap.Contains(FlowerWorldCoordinates)) {
		SpawnedFlowerMap[FlowerWorldCoordinates].Add(FlowerActor);
	} else {
		// If not, create a new array with the new flower
		SpawnedFlowerMap.Add(FlowerWorldCoordinates, TArray<UCustomProceduralMeshComponent*>({ FlowerActor }));
	}

	FlowerMapSemaphore->Release();
}

void UWorldTerrainSettings::AddSpawnedNpc(const FIntPoint& npcWorldCoordinates, ABasicNPC* npcActor) {
	NpcMapSemaphore->Acquire();

	// If the coordinate of the chunk exists, add the new NPC to the existing array
	if (SpawnedNpcMap.Contains(npcWorldCoordinates)) {
		SpawnedNpcMap[npcWorldCoordinates].Add(npcActor);
	} else {
		// If not, create a new array with the new NPC
		SpawnedNpcMap.Add(npcWorldCoordinates, TArray<ABasicNPC*>({ npcActor }));
	}

	NpcMapSemaphore->Release();
}

const TMap<FIntPoint, TArray<ATree*>>& UWorldTerrainSettings::GetSpawnedTreesMap() const {
	return SpawnedTreesMap;
}

TArray<ATree*> UWorldTerrainSettings::GetAndRemoveTreeFromMap(const FIntPoint& TreeWorldCoordinates) {
	TArray<ATree*> RemovedTrees;  // Array to hold the remaining trees at the location

	TreeMapSemaphore->Acquire();

	// Check if the map contains the coordinates
	if (SpawnedTreesMap.Contains(TreeWorldCoordinates)) {
		// Get and remove the array of trees at this location if it's not empty
		if (!SpawnedTreesMap[TreeWorldCoordinates].IsEmpty()) {
			RemovedTrees = SpawnedTreesMap.FindAndRemoveChecked(TreeWorldCoordinates);
			//RemovedTrees = *SpawnedTreesMap.Find(TreeWorldCoordinates);
		}
	}

	TreeMapSemaphore->Release();
	return RemovedTrees;
}
TArray<UCustomProceduralMeshComponent*> UWorldTerrainSettings::GetAndRemoveGrassFromMap(const FIntPoint& GrassWorldCoordinates) {  
	TArray<UCustomProceduralMeshComponent*> RemovedGrass;  // Array to hold the remaining grass at the location  

	GrassMapSemaphore->Acquire();  

	// Check if the map contains the coordinates  
	if (SpawnedGrassMap.Contains(GrassWorldCoordinates)) {  
		// Get and remove the array of grass at this location if it's not empty  
		if (!SpawnedGrassMap[GrassWorldCoordinates].IsEmpty()) {  
			RemovedGrass = SpawnedGrassMap.FindAndRemoveChecked(GrassWorldCoordinates);  
			SpawnedGrassMap.Remove(GrassWorldCoordinates);  
		} 
	}  

	GrassMapSemaphore->Release();  
	return RemovedGrass;
}

TArray<UCustomProceduralMeshComponent*> UWorldTerrainSettings::GetAndRemoveFlowerFromMap(const FIntPoint& FlowerWorldCoordinates) {
	TArray<UCustomProceduralMeshComponent*> RemovedFlower;  // Array to hold the remaining flower at the location

	FlowerMapSemaphore->Acquire();

	// Check if the map contains the coordinates
	if (SpawnedFlowerMap.Contains(FlowerWorldCoordinates)) {
		// Get and remove the array of flower at this location if it's not empty
		if (!SpawnedFlowerMap[FlowerWorldCoordinates].IsEmpty()) {
			RemovedFlower = SpawnedFlowerMap.FindAndRemoveChecked(FlowerWorldCoordinates);
			SpawnedFlowerMap.Remove(FlowerWorldCoordinates);
		}
	}

	FlowerMapSemaphore->Release();
	return RemovedFlower;
}

TArray<ABasicNPC*> UWorldTerrainSettings::GetAndRemoveNpcFromMap(const FIntPoint& npcWorldCoordinates) {
	TArray<ABasicNPC*> RemovedNpcs;  // Array to hold the remaining NPCs at the location

	NpcMapSemaphore->Acquire();

	// Check if the map contains the chunk world coordinates
	if (SpawnedNpcMap.Contains(npcWorldCoordinates)) {
		// Get and remove the array of NPCs at this location if it's not empty
		if (!SpawnedNpcMap[npcWorldCoordinates].IsEmpty()) {
			RemovedNpcs = SpawnedNpcMap.FindAndRemoveChecked(npcWorldCoordinates);
			// TODO If using LOD system, remove the key as well (like for the Flower/Grass
		}
	}

	NpcMapSemaphore->Release();
	return RemovedNpcs;
}

void UWorldTerrainSettings::RemoveTreeFromMap(const FIntPoint& TreeWorldCoordinates) {
	TreeMapSemaphore->Acquire();

	if (SpawnedTreesMap.Contains(TreeWorldCoordinates)) {
		SpawnedTreesMap.Remove(TreeWorldCoordinates);
	}

	TreeMapSemaphore->Release();
}

void UWorldTerrainSettings::CheckAndReturnGrassNotInRange(TArray<FIntPoint>& coordinates, TQueue<UCustomProceduralMeshComponent*>* GrassActorsToRemove) {
   GrassMapSemaphore->Acquire();  

   // Iterate over the keys of SpawnedGrassMap  
   for (TMap<FIntPoint, TArray<UCustomProceduralMeshComponent*>>::TIterator GrassMapIterator = SpawnedGrassMap.CreateIterator(); GrassMapIterator; ++GrassMapIterator) {  
      const FIntPoint& Key = GrassMapIterator.Key();  

      // Check if the key is not in the provided coordinates  
      if (!coordinates.Contains(Key)) {  
		  // Add each grass component to the queue  
		  for (UCustomProceduralMeshComponent* GrassActor : GrassMapIterator.Value()) {
			  GrassActorsToRemove->Enqueue(GrassActor);
		  }

          // Remove the key from the map  
          GrassMapIterator.RemoveCurrent();  
      }  
   }

   GrassMapSemaphore->Release();  
}

void UWorldTerrainSettings::CheckAndReturnFlowersNotInRange(TArray<FIntPoint>& coordinates, TQueue<UCustomProceduralMeshComponent*>* FlowerActorsToRemove) {
	FlowerMapSemaphore->Acquire();

	// Iterate over the keys of SpawnedFlowerMap  
	for (TMap<FIntPoint, TArray<UCustomProceduralMeshComponent*>>::TIterator FlowerMapIterator = SpawnedFlowerMap.CreateIterator(); FlowerMapIterator; ++FlowerMapIterator) {
		const FIntPoint& Key = FlowerMapIterator.Key();

		// Check if the key is not in the provided coordinates  
		if (!coordinates.Contains(Key)) {
			// Add each flower component to the queue  
			for (UCustomProceduralMeshComponent* FlowerActor : FlowerMapIterator.Value()) {
				FlowerActorsToRemove->Enqueue(FlowerActor);
			}

			// Remove the key from the map  
			FlowerMapIterator.RemoveCurrent();
		}
	}

	FlowerMapSemaphore->Release();
}

void UWorldTerrainSettings::CheckAndReturnTreesNotInRange(TArray<FIntPoint>& coordinates, TQueue<ATree*>* TreeActorsToRemove) {
	TreeMapSemaphore->Acquire();

	// Iterate over the keys of SpawnedTreesMap  
	for (TMap<FIntPoint, TArray<ATree*>>::TIterator TreeMapIterator = SpawnedTreesMap.CreateIterator(); TreeMapIterator; ++TreeMapIterator) {
		const FIntPoint& Key = TreeMapIterator.Key();

		// Check if the key is not in the provided coordinates  
		if (!coordinates.Contains(Key)) {
			// Add each tree actor to the queue  
			for (ATree* TreeActor : TreeMapIterator.Value()) {
				TreeActorsToRemove->Enqueue(TreeActor);
			}

			// Remove the key from the map  
			TreeMapIterator.RemoveCurrent();
		}
	}
	TreeMapSemaphore->Release();
}

void UWorldTerrainSettings::CheckAndReturnNpcsNotInRange(TArray<FIntPoint>& coordinates, TQueue<ABasicNPC*>* NpcActorsToRemove) {
	NpcMapSemaphore->Acquire();

	// Iterate over the keys of SpawnedNpcMap  
	for (TMap<FIntPoint, TArray<ABasicNPC*>>::TIterator NpcMapIterator = SpawnedNpcMap.CreateIterator(); NpcMapIterator; ++NpcMapIterator) {
		const FIntPoint& Key = NpcMapIterator.Key();

		// Check if the key is not in the provided coordinates  
		if (!coordinates.Contains(Key)) {
			// Add each NPC actor to the queue  
			for (ABasicNPC* NpcActor : NpcMapIterator.Value()) {
				NpcActorsToRemove->Enqueue(NpcActor);
			}
			// Remove the key from the map  
			NpcMapIterator.RemoveCurrent();
		}
	}
	NpcMapSemaphore->Release();
}

void UWorldTerrainSettings::RemoveSingleGrassFromMap(UCustomProceduralMeshComponent* grass) {
    GrassMapSemaphore->Acquire();

    if (SpawnedGrassMap.Contains(grass->ObjectWorldCoords)) {
        TArray<UCustomProceduralMeshComponent*>& GrassArray = SpawnedGrassMap[grass->ObjectWorldCoords];
        GrassArray.Remove(grass);
        if (GrassArray.Num() == 0) {
            SpawnedGrassMap.Remove(grass->ObjectWorldCoords);
        }
    }

    GrassMapSemaphore->Release();
}

void UWorldTerrainSettings::RemoveSingleFlowerFromMap(UCustomProceduralMeshComponent* flower) {
	FlowerMapSemaphore->Acquire();

	if (SpawnedFlowerMap.Contains(flower->ObjectWorldCoords)) {
		TArray<UCustomProceduralMeshComponent*>& FlowerArray = SpawnedFlowerMap[flower->ObjectWorldCoords];
		FlowerArray.Remove(flower);
		if (FlowerArray.Num() == 0) {
			SpawnedFlowerMap.Remove(flower->ObjectWorldCoords);
		}
	}

	FlowerMapSemaphore->Release();
}

void UWorldTerrainSettings::RemoveSingleNpcFromMap(ABasicNPC* npc) {
	NpcMapSemaphore->Acquire();

	if (SpawnedNpcMap.Contains(npc->GetNpcWorldLocation())) {
		TArray<ABasicNPC*>& NpcArray = SpawnedNpcMap[npc->GetNpcWorldLocation()];
		NpcArray.Remove(npc);
		if (NpcArray.Num() == 0) {
			SpawnedNpcMap.Remove(npc->GetNpcWorldLocation());
		}
	}

	NpcMapSemaphore->Release();
}

void UWorldTerrainSettings::AddChunkToRemoveCollision(ABinaryChunk* actor) {
	RemoveCollisionChunksSemaphore->Acquire();
	RemoveCollisionChunks.Add(actor);
	RemoveCollisionChunksSemaphore->Release();
}

void UWorldTerrainSettings::AddTreeToRemoveCollision(ATree* actor) {
	RemoveCollisionTreesSemaphore->Acquire();
	RemoveCollisionTrees.Add(actor);
	RemoveCollisionTreesSemaphore->Release();
}

void UWorldTerrainSettings::AddChunkToEnableCollision(ABinaryChunk* actor) {
	AddCollisionChunksSemaphore->Acquire();
	AddCollisionChunks.Add(actor);
	AddCollisionChunksSemaphore->Release();
}

void UWorldTerrainSettings::AddTreeToEnableCollision(ATree* actor) {
	AddCollisionTreesSemaphore->Acquire();
	AddCollisionTrees.Add(actor);
	AddCollisionTreesSemaphore->Release();
}

ABinaryChunk* UWorldTerrainSettings::GetChunkToRemoveCollision() {
	// Return and remove the first actor
	if (!RemoveCollisionChunks.IsEmpty()) {
		ABinaryChunk* ActorToRemove = RemoveCollisionChunks[0];
		RemoveCollisionChunks.RemoveAt(0);
		return ActorToRemove;
	}

	return nullptr;
}

ATree* UWorldTerrainSettings::GetTreeToRemoveCollision() {
	// Return and remove the first actor
	if (!RemoveCollisionTrees.IsEmpty()) {
		ATree* ActorToRemove = RemoveCollisionTrees[0];
		RemoveCollisionTrees.RemoveAt(0);
		return ActorToRemove;
	}

	return nullptr;
}

ABinaryChunk* UWorldTerrainSettings::GetChunkToEnableCollision() {
	// Return and remove the first actor
	if (!AddCollisionChunks.IsEmpty()) {
		ABinaryChunk* ActorToRemove = AddCollisionChunks[0];
		AddCollisionChunks.RemoveAt(0);
		return ActorToRemove;
	}

	return nullptr;
}

ATree* UWorldTerrainSettings::GetTreeToEnableCollision() {
	// Return and remove the first actor
	if (!AddCollisionTrees.IsEmpty()) {
		ATree* ActorToRemove = AddCollisionTrees[0];
		AddCollisionTrees.RemoveAt(0);
		return ActorToRemove;
	}

	return nullptr;
}

// Testing method that checks for duplicated Chunk (AActor) pointers  in SpawnChunkMap
void UWorldTerrainSettings::CheckForDuplicateActorPointers() {
	// Create a set to track encountered actors
	TSet<AActor*> SeenActors;

	// Iterate over the map values
	for (const TPair<FIntPoint, AActor*>& Pair : SpawnedChunksMap) {
		AActor* Actor = Pair.Value;

		// Check if the actor is already in the set
		if (SeenActors.Contains(Actor)) {
			// Duplicate found
			UE_LOG(LogTemp, Error, TEXT("Duplicate actor detected: %s"), *Actor->GetName());
		}

		// Add the actor to the set
		SeenActors.Add(Actor);
	}
}

void UWorldTerrainSettings::CheckIfActorIsNullOrPendingKill() {
	for (TPair<FIntPoint, AActor*>& Elem : SpawnedChunksMap) {
		if (!IsValid(Elem.Value) || Elem.Value->IsPendingKillPending()) {
			UE_LOG(LogTemp, Error, TEXT("CheckIfActorIsNullOrPendingKill(): Invalid chunk at coordinates: (%d, %d)"), Elem.Key.X, Elem.Key.Y);
		}
	}
}

void UWorldTerrainSettings::CheckForDuplicateWorldCoordinates() {
	TSet<FIntPoint> uniqueCoordinates; // A set to store unique FIntPoint values
	TArray<FIntPoint> duplicateCoordinates; // Array to track duplicates

	for (const TPair<FIntPoint, AActor*>& Elem : SpawnedChunksMap) {
		const FIntPoint& chunkCoord = Elem.Key;

		// Check if the FIntPoint is already in the set
		if (uniqueCoordinates.Contains(chunkCoord)) {
			duplicateCoordinates.Add(chunkCoord);
		} else {
			uniqueCoordinates.Add(chunkCoord); 
		}
	}

	// Log duplicate coordinates if any are found
	if (duplicateCoordinates.Num() > 0) {
		FString duplicatesLog = TEXT("Duplicate World Coordinates Found:\n");
		for (const FIntPoint& coord : duplicateCoordinates) {
			duplicatesLog += FString::Printf(TEXT("(%d, %d)\n"), coord.X, coord.Y);
		}
		UE_LOG(LogTemp, Warning, TEXT("%s"), *duplicatesLog);
	}
}

void UWorldTerrainSettings::CheckNumberOfElements() {
	if (SpawnedChunksMap.Num() < 100) {
		printMapElements("CheckNumberOfElements(): ");
		UE_LOG(LogTemp, Error, TEXT("Map has less than 100 items!: Map size: %d"), SpawnedChunksMap.Num());
	}
	else if (SpawnedChunksMap.Num() > 101) {
		printMapElements("CheckNumberOfElements(): ");
		UE_LOG(LogTemp, Error, TEXT("Map has more than 100 items!: Map size: %d"), SpawnedChunksMap.Num());
	}
}

void UWorldTerrainSettings::initializePerlinNoise(TObjectPtr<FastNoiseLite>& noise) {
	noise = new FastNoiseLite(NoiseSeed);
	noise->SetNoiseType(FastNoiseLite::NoiseType_Perlin);
	noise->SetFractalType(FastNoiseLite::FractalType_FBm);
}

void UWorldTerrainSettings::applyDomainWarpSettings(TObjectPtr<FastNoiseLite>& domainWarp, const int& settingsIndex) {
	// Adding domain warp settings
	domainWarp->SetDomainWarpAmp(PNSR->noiseMapSettings[settingsIndex].DomainWarpAmp);
	domainWarp->SetFrequency(PNSR->noiseMapSettings[settingsIndex].DomainWarpFrequencies);
	domainWarp->SetFractalOctaves(PNSR->noiseMapSettings[settingsIndex].DomainWarpOctaves);
	domainWarp->SetFractalLacunarity(PNSR->noiseMapSettings[settingsIndex].DomainWarpLacunarity);
	domainWarp->SetFractalGain(PNSR->noiseMapSettings[settingsIndex].DomainWarpGain);
}

// Use the settingsIndex to apply the correct noise settings from PerlinNoiseSettings.cpp
void UWorldTerrainSettings::applyPerlinNoiseSettings(TObjectPtr<FastNoiseLite>& noise, const int& settingsIndex) {
	// Set perlin noise settings
	noise->SetFractalOctaves(PNSR->noiseMapSettings[settingsIndex].Octaves);
	noise->SetFrequency(PNSR->noiseMapSettings[settingsIndex].Frequencies);
	noise->SetFractalLacunarity(PNSR->noiseMapSettings[settingsIndex].Lacunarity);
	noise->SetFractalGain(PNSR->noiseMapSettings[settingsIndex].Gain);
	noise->SetFractalWeightedStrength(PNSR->noiseMapSettings[settingsIndex].WeightedStrength);
}

// Use the settingsIndex to apply the correct noise settings from PerlinNoiseSettings.cpp
void UWorldTerrainSettings::initializeDomainWarpNoise(TObjectPtr<FastNoiseLite>& domainWarp) {
	domainWarp = new FastNoiseLite();
	domainWarp->SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
	domainWarp->SetFractalType(FastNoiseLite::FractalType_DomainWarpProgressive);
}

// Set the perlin noise data reference and iniaitlize the FastNoise objects with their settings
void UWorldTerrainSettings::SetPerlinNoiseSettings(APerlinNoiseSettings* InPerlinNoiseSettings) {
	PerlinNoiseSettingsRef = InPerlinNoiseSettings;

	// Declaring the 3 noise objects used to create the terrain
	initializePerlinNoise(continentalness);
	initializePerlinNoise(erosion);
	initializePerlinNoise(peaksAndValleys);

	// Declaring the 3 noise domain warp objects to modify the terrain
	initializeDomainWarpNoise(continentalnessDW);
	initializeDomainWarpNoise(erosionDW);
	initializeDomainWarpNoise(peaksAndValleysDW);

	// Apply settings
	applyPerlinNoiseSettings(continentalness, 0);
	applyPerlinNoiseSettings(erosion, 1);
	applyPerlinNoiseSettings(peaksAndValleys, 2);

	applyDomainWarpSettings(continentalnessDW, 0);
	applyDomainWarpSettings(erosionDW, 1);
	applyDomainWarpSettings(peaksAndValleysDW, 2);
}

void UWorldTerrainSettings::ApplyDomainWarpToCoords(float& noisePositionX, float& noisePositionZ, TObjectPtr<FastNoiseLite> noise) {
	noise->DomainWarp(noisePositionX, noisePositionZ);
}

float UWorldTerrainSettings::GetNoiseAtCoords(float& noisePositionX, float& noisePositionZ, TObjectPtr<FastNoiseLite> noise) {
	return noise->GetNoise(noisePositionX, noisePositionZ) + 1;;
}
