// Fill out your copyright notice in the Description page of Project Settings.

#include "ChunksLocationRunnable.h"
#include "..\SingleChunk\BinaryChunk.h"
#include "..\Vegetation\Trees\Tree.h"
#include "..\TerrainSettings\WorldTerrainSettings.h"
#include "..\ChunkData\ChunkLocationData.h"

ChunksLocationRunnable::ChunksLocationRunnable(FVector PlayerPosition, UWorldTerrainSettings* InWorldTerrainSettingsRef, UChunkLocationData* InChunkLocationDataRef) : PlayerPosition(PlayerPosition), isRunning(false), isTaskComplete(false) {
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
		// Add spawn points for Chunks
		WTSR->UpdateChunkSemaphore->Acquire();
		UpdateSpawnPoints(CHUNKS);
		WTSR->UpdateChunkSemaphore->Release();

		UpdateSpawnPoints(VEGETATION);

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
				CLDR->addChunksToDestroyPosition(oldChunkCoords);
				CLDR->addChunksToSpawnPosition(FVoxelObjectLocationData(ChunkPosition, newChunkCoords));
			} else if (SpawnType == VEGETATION) {
				CLDR->AddVegetationChunkSpawnPosition(newChunkCoords);
				CLDR->RemoveVegetationChunkSpawnPosition(oldChunkCoords);
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
				CLDR->addChunksToDestroyPosition(oldChunkCoords);
				CLDR->addChunksToSpawnPosition(FVoxelObjectLocationData(ChunkPosition, newChunkCoords));
			} else if (SpawnType == VEGETATION) {
				CLDR->AddVegetationChunkSpawnPosition(newChunkCoords);
				CLDR->RemoveVegetationChunkSpawnPosition(oldChunkCoords);
			}
		}
	}

	// Update the initial position for the next frame
	WTSR->updateInitialPlayerPosition(PlayerPosition);
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



