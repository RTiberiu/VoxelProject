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
		// Spawn new chunks
		WTSR->UpdateChunkSemaphore->Acquire();
		UpdateChunks();
		WTSR->UpdateChunksCollision(PlayerPosition);
		WTSR->UpdateTreeCollisions(PlayerPosition);
		WTSR->UpdateChunkSemaphore->Release();

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

void ChunksLocationRunnable::UpdateChunks() {
	FIntPoint PlayerChunkCoords = GetChunkCoordinates(PlayerPosition);
	FIntPoint InitialChunkCoords = GetChunkCoordinates(WTSR->getInitialPlayerPosition());

	// Add and remove chunks on the X axis 
	if (PlayerChunkCoords.X != InitialChunkCoords.X) {

		int lastRowX{ 0 };
		int newRowX{ 0 };
		if (PlayerChunkCoords.X > InitialChunkCoords.X) {
			// Get the first row by adding the draw distance to the player's X
			UE_LOG(LogTemp, Warning, TEXT("Moved right"));
			lastRowX = PlayerChunkCoords.X - WTSR->DrawDistance - 1;
			newRowX = PlayerChunkCoords.X + WTSR->DrawDistance - 1;
		} else {
			// Get the last row by substracting the draw distance from the the initial X
			UE_LOG(LogTemp, Warning, TEXT("Moved left"));
			lastRowX = InitialChunkCoords.X + WTSR->DrawDistance - 1;
			newRowX = InitialChunkCoords.X - WTSR->DrawDistance - 1;
		}

		// Add new chunks and remove old chunks based on the player's new X position
		const int firstIndexChunkZ = InitialChunkCoords.Y - WTSR->DrawDistance;
		const int lastIndexChunkZ = InitialChunkCoords.Y + WTSR->DrawDistance;

		// Loop and remove the entire row of chunks 
		for (int z = firstIndexChunkZ; z < lastIndexChunkZ; z++) {
			FIntPoint oldChunkCoords = FIntPoint(lastRowX, z);

			CLDR->addChunksToDestroyPosition(oldChunkCoords);
			
			// Adding new row 
			FIntPoint newChunkCoords = FIntPoint(newRowX, z);
			FVector ChunkPosition = FVector(newRowX * WTSR->chunkSize * WTSR->UnrealScale, z * WTSR->chunkSize * WTSR->UnrealScale, 0);

			CLDR->addChunksToSpawnPosition(FVoxelObjectLocationData(ChunkPosition, newChunkCoords));
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
			UE_LOG(LogTemp, Warning, TEXT("Moved up"));
			lastRowZ = InitialChunkCoords.Y - WTSR->DrawDistance;
			newRowZ = InitialChunkCoords.Y + WTSR->DrawDistance;
		} else {
			// Get the last row by substracting the draw distance from the the initial Z
			UE_LOG(LogTemp, Warning, TEXT("Moved down"));
			lastRowZ = PlayerChunkCoords.Y + WTSR->DrawDistance;
			newRowZ = PlayerChunkCoords.Y - WTSR->DrawDistance;
		}

		const int firstIndexChunkX = PlayerChunkCoords.X - WTSR->DrawDistance;
		const int lastIndexChunkX = PlayerChunkCoords.X + WTSR->DrawDistance;

		// Loop and remove the entire row of chunks 
		for (int x = firstIndexChunkX; x < lastIndexChunkX; x++) {
			FIntPoint oldChunkCoords = FIntPoint(x, lastRowZ);

			CLDR->addChunksToDestroyPosition(oldChunkCoords);

			FIntPoint newChunkCoords = FIntPoint(x, newRowZ);
			FVector ChunkPosition = FVector(x * WTSR->chunkSize * WTSR->UnrealScale, newRowZ * WTSR->chunkSize * WTSR->UnrealScale, 0);

			CLDR->addChunksToSpawnPosition(FVoxelObjectLocationData(ChunkPosition, newChunkCoords));
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



