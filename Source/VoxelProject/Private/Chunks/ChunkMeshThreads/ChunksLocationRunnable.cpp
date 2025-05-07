// Fill out your copyright notice in the Description page of Project Settings.

#include "ChunksLocationRunnable.h"
#include "..\SingleChunk\BinaryChunk.h"
#include "..\Vegetation\Trees\Tree.h"
#include "..\TerrainSettings\WorldTerrainSettings.h"
#include "..\ChunkData\ChunkLocationData.h"

ChunksLocationRunnable::ChunksLocationRunnable(
	FVector PlayerPosition,
	UWorldTerrainSettings* InWorldTerrainSettingsRef,
	UChunkLocationData* InChunkLocationDataRef,
	TQueue<UCustomProceduralMeshComponent*>* InGrassActorsToRemove,
	TQueue<UCustomProceduralMeshComponent*>* InFlowerActorsToRemove,
	TQueue<ATree*>* InTreeActorsToRemove,
	TQueue<ABasicNPC*>* InNpcActorsToRemove
) : 
	PlayerPosition(PlayerPosition), 
	isRunning(false), 
	isTaskComplete(false),
	GrassActorsToRemove(InGrassActorsToRemove),
	FlowerActorsToRemove(InFlowerActorsToRemove),
	TreeActorsToRemove(InTreeActorsToRemove),
	NpcActorsToRemove(InNpcActorsToRemove)
{
	WorldTerrainSettingsRef = InWorldTerrainSettingsRef;
	ChunkLocationDataRef = InChunkLocationDataRef;
}

ChunksLocationRunnable::~ChunksLocationRunnable() {
}

bool ChunksLocationRunnable::Init() {
	isRunning = true;
	isTaskComplete = false;
	return true;
}

uint32 ChunksLocationRunnable::Run() {
	while (isRunning) {
		// Add spawn/despawn points for Chunks
		WTSR->UpdateChunkSemaphore->Acquire();
		UpdateSpawnPoints(CHUNKS);
		WTSR->UpdateChunkSemaphore->Release();

		// TODO These calls should be also protected by a semaphore.
		// This currently causes the pointer crash when despawning vegetation.
		
		// Add spawn/despawn points for vegetation, trees, and NPCs
		UpdateSpawnPoints(VEGETATION);
		UpdateSpawnPoints(TREES);
		UpdateSpawnPoints(NPCS);

		// Update the initial position for the next frame
		WTSR->UpdateInitialPlayerPosition(PlayerPosition);

		// Update Chunks and Trees collision
		WTSR->UpdateChunksCollision(PlayerPosition);
		WTSR->UpdateTreeCollisions(PlayerPosition);

		isTaskComplete = true;
		isRunning = false;
	}
	return 0;
}


void ChunksLocationRunnable::Stop() {
	isRunning = false;
}

void ChunksLocationRunnable::Exit() {
}

FThreadSafeBool ChunksLocationRunnable::IsTaskComplete() const {
	return isTaskComplete;
}

void ChunksLocationRunnable::UpdateSpawnPoints(SpawnPointType SpawnType) {
	FIntPoint PlayerChunkCoords = GetChunkCoordinates(PlayerPosition);
	FIntPoint InitialChunkCoords = GetChunkCoordinates(WTSR->getInitialPlayerPosition());

	uint8_t DrawDistance;
	if (SpawnType == CHUNKS) {
		DrawDistance = WTSR->DrawDistance;
	} else if (SpawnType == VEGETATION) {
		DrawDistance = WTSR->VegetationDrawDistance;
	} else if (SpawnType == TREES) {
		DrawDistance = WTSR->TreeDrawDistance;
	} else if (SpawnType == NPCS) {
		DrawDistance = WTSR->NpcDrawDistance;
	}

	// Add and remove chunks on the X axis 
	if (PlayerChunkCoords.X != InitialChunkCoords.X) {
		int lastRowX{ 0 };
		int newRowX{ 0 };
		if (PlayerChunkCoords.X > InitialChunkCoords.X) {
			// Get the first row by adding the draw distance to the player's X
			//UE_LOG(LogTemp, Warning, TEXT("Moved right")); // TESTING 
			lastRowX = PlayerChunkCoords.X - DrawDistance - 1;
			newRowX = PlayerChunkCoords.X + DrawDistance - 1;
		} else {
			// Get the last row by substracting the draw distance from the the initial X
			//UE_LOG(LogTemp, Warning, TEXT("Moved left")); // TESTING 
			lastRowX = InitialChunkCoords.X + DrawDistance - 1;
			newRowX = InitialChunkCoords.X - DrawDistance - 1;
		}

		// Add new chunks and remove old chunks based on the player's new X position
		const int firstIndexChunkZ = InitialChunkCoords.Y - DrawDistance;
		const int lastIndexChunkZ = InitialChunkCoords.Y + DrawDistance;

		// Loop and remove the entire row of chunks 
		for (int z = firstIndexChunkZ; z < lastIndexChunkZ; z++) {
			FIntPoint oldChunkCoords = FIntPoint(lastRowX, z);

			// Adding new row 
			FIntPoint newChunkCoords = FIntPoint(newRowX, z);

			if (SpawnType == CHUNKS) {
				FVector ChunkPosition = FVector(newRowX * WTSR->chunkSize * WTSR->UnrealScale, z * WTSR->chunkSize * WTSR->UnrealScale, 0);
				CLDR->AddChunksToDestroyPosition(oldChunkCoords);
				CLDR->AddChunksToSpawnPosition(FVoxelObjectLocationData(ChunkPosition, newChunkCoords));
			} else if (SpawnType == VEGETATION) {
				CLDR->AddVegetationChunkSpawnPosition(newChunkCoords);
				CLDR->RemoveVegetationChunkSpawnPosition(oldChunkCoords);

				// Add the spawned vegetation mesh component to the lists, 
				// so they can be removed over multiple frames
                TArray<UCustomProceduralMeshComponent*> grassToRemove = WTSR->GetAndRemoveGrassFromMap(oldChunkCoords);  
                for (UCustomProceduralMeshComponent* GrassActor : grassToRemove) {  
					if (GrassActor == nullptr) continue;
					GrassActorsToRemove->Enqueue(GrassActor);
                }

                TArray<UCustomProceduralMeshComponent*> flowerToRemove = WTSR->GetAndRemoveFlowerFromMap(oldChunkCoords);  
                for (UCustomProceduralMeshComponent* FlowerActor : flowerToRemove) {  
					if (FlowerActor == nullptr) continue;
					FlowerActorsToRemove->Enqueue(FlowerActor);  
                }
			} else if (SpawnType == TREES) {
				CLDR->AddTreeChunkSpawnPosition(newChunkCoords);
				CLDR->RemoveTreeChunkSpawnPosition(oldChunkCoords);

				TArray<ATree*> treesToRemove = WTSR->GetAndRemoveTreeFromMap(oldChunkCoords);
				for (ATree* TreeActor : treesToRemove) {
					if (TreeActor == nullptr) continue;
					TreeActorsToRemove->Enqueue(TreeActor);
				}
			} else if (SpawnType == NPCS) {
				CLDR->AddNpcChunkSpawnPosition(newChunkCoords);
				CLDR->RemoveNpcChunkSpawnPosition(oldChunkCoords);

				// Add spawned NPCs to a remove list to remove over multiple frames
				TArray<ABasicNPC*> npcsToRemove = WTSR->GetAndRemoveNpcFromMap(oldChunkCoords);
				for (ABasicNPC* NpcActor : npcsToRemove) {
					if (NpcActor == nullptr) continue;
					NpcActorsToRemove->Enqueue(NpcActor);
				}
			}
		} 
	}
	

	// Update the initial position for the next frame
	PlayerChunkCoords = GetChunkCoordinates(PlayerPosition);
	InitialChunkCoords = GetChunkCoordinates(WTSR->getInitialPlayerPosition());

	// Add and remove chunks on the Y axis 
	if (PlayerChunkCoords.Y != InitialChunkCoords.Y) {
		// Add new chunks and remove old chunks based on the player's new Y position
		int lastRowZ{ 0 };
		int newRowZ{ 0 };
		int lastTreeRowZ{ 0 };
		int newTreeRowZ{ 0 };
		if (PlayerChunkCoords.Y > InitialChunkCoords.Y) {
			// Get the first row by adding the draw distance to the player's Z
			lastRowZ = InitialChunkCoords.Y - DrawDistance;
			newRowZ = InitialChunkCoords.Y + DrawDistance;
		} else {
			// Get the last row by substracting the draw distance from the the initial Z
			lastRowZ = PlayerChunkCoords.Y + DrawDistance;
			newRowZ = PlayerChunkCoords.Y - DrawDistance;
		}

		const int firstIndexChunkX = PlayerChunkCoords.X - DrawDistance;
		const int lastIndexChunkX = PlayerChunkCoords.X + DrawDistance;

		// Loop and remove the entire row of chunks 
		for (int x = firstIndexChunkX; x < lastIndexChunkX; x++) {
			FIntPoint oldChunkCoords = FIntPoint(x, lastRowZ);
			FIntPoint newChunkCoords = FIntPoint(x, newRowZ);

			if (SpawnType == CHUNKS) {
				FVector ChunkPosition = FVector(x * WTSR->chunkSize * WTSR->UnrealScale, newRowZ * WTSR->chunkSize * WTSR->UnrealScale, 0);
				CLDR->AddChunksToDestroyPosition(oldChunkCoords);
				CLDR->AddChunksToSpawnPosition(FVoxelObjectLocationData(ChunkPosition, newChunkCoords));
			} else if (SpawnType == VEGETATION) {
				CLDR->AddVegetationChunkSpawnPosition(newChunkCoords);
				CLDR->RemoveVegetationChunkSpawnPosition(oldChunkCoords);

				// Add the spawned vegetation mesh component to the lists, 
				// so they can be removed over multiple frames
				TArray<UCustomProceduralMeshComponent*> grassToRemove = WTSR->GetAndRemoveGrassFromMap(oldChunkCoords);
				for (UCustomProceduralMeshComponent* GrassActor : grassToRemove) {
					if (GrassActor == nullptr) continue;
					GrassActorsToRemove->Enqueue(GrassActor);
				}

				TArray<UCustomProceduralMeshComponent*> flowerToRemove = WTSR->GetAndRemoveFlowerFromMap(oldChunkCoords);
				for (UCustomProceduralMeshComponent* FlowerActor : flowerToRemove) {
					if (FlowerActor == nullptr) continue;
					FlowerActorsToRemove->Enqueue(FlowerActor);
				}
			} else if (SpawnType == TREES) {
				CLDR->AddTreeChunkSpawnPosition(newChunkCoords);
				CLDR->RemoveTreeChunkSpawnPosition(oldChunkCoords);

				TArray<ATree*> treesToRemove = WTSR->GetAndRemoveTreeFromMap(oldChunkCoords);
				for (ATree* TreeActor : treesToRemove) {
					if (TreeActor == nullptr) continue;
					TreeActorsToRemove->Enqueue(TreeActor);
				}
			} else if (SpawnType == NPCS) {
				CLDR->AddNpcChunkSpawnPosition(newChunkCoords);
				CLDR->RemoveNpcChunkSpawnPosition(oldChunkCoords);

				// Add spawned NPCs to a remove list to remove over multiple frames
				TArray<ABasicNPC*> npcsToRemove = WTSR->GetAndRemoveNpcFromMap(oldChunkCoords);
				for (ABasicNPC* NpcActor : npcsToRemove) {
					if (NpcActor == nullptr) continue;
					NpcActorsToRemove->Enqueue(NpcActor);
				}
			}
		}
	}
}

FIntPoint ChunksLocationRunnable::GetChunkCoordinates(FVector Position) const {
	const int32 ChunkX = FMath::FloorToInt(Position.X / (WTSR->chunkSize * WTSR->UnrealScale));
	const int32 ChunkY = FMath::FloorToInt(Position.Y / (WTSR->chunkSize * WTSR->UnrealScale));
	return FIntPoint(ChunkX, ChunkY);
}

void ChunksLocationRunnable::SetWorldTerrainSettings(UWorldTerrainSettings* InWorldTerrainSettings) {
	WorldTerrainSettingsRef = InWorldTerrainSettings;
}

void ChunksLocationRunnable::SetChunkLocationData(UChunkLocationData* InChunkLocationData) {
	ChunkLocationDataRef = InChunkLocationData;
}



