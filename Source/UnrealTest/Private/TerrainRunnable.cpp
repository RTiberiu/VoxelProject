// Fill out your copyright notice in the Description page of Project Settings.

#include "TerrainRunnable.h"
#include "WorldTerrainSettings.h"
#include "ChunkLocationData.h"

TerrainRunnable::TerrainRunnable(FVector PlayerPosition, UWorldTerrainSettings* InWorldTerrainSettingsRef, UChunkLocationData* InChunkLocationDataRef) : PlayerPosition(PlayerPosition), isRunning(false), isTaskComplete(false) {
	WorldTerrainSettingsRef = InWorldTerrainSettingsRef;
	ChunkLocationDataRef = InChunkLocationDataRef;
}

TerrainRunnable::~TerrainRunnable() {
}

bool TerrainRunnable::Init() {
	isRunning = true;
	isTaskComplete = false;
	return true;
}

uint32 TerrainRunnable::Run() {
	while (isRunning) {
		// Spawn new chunks
		WTSR->UpdateChunkSemaphore->Acquire();
		UpdateChunks();
		WTSR->UpdateChunkSemaphore->Release();

		isTaskComplete = true;
		isRunning = false;
	}
	return 0;
}

void TerrainRunnable::Stop() {
	isRunning = false;
}

void TerrainRunnable::Exit() {
}

FThreadSafeBool TerrainRunnable::IsTaskComplete() const {
	return isTaskComplete;
}

void TerrainRunnable::UpdateChunks() {
	FIntPoint PlayerChunkCoords = GetChunkCoordinates(PlayerPosition);
	FIntPoint InitialChunkCoords = GetChunkCoordinates(WTSR->getInitialPlayerPosition());

	// Add and remove chunks on the X axis 
	if (PlayerChunkCoords.X != InitialChunkCoords.X) {

		int lastRowX{ 0 };
		int newRowX{ 0 };
		if (PlayerChunkCoords.X > InitialChunkCoords.X) {
			// Get the first row by adding the draw distance to the player's X
			lastRowX = PlayerChunkCoords.X - WTSR->DrawDistance;
			newRowX = PlayerChunkCoords.X + WTSR->DrawDistance;
		} else {
			// Get the last row by substracting the draw distance from the the initial X
			lastRowX = InitialChunkCoords.X + WTSR->DrawDistance;
			newRowX = InitialChunkCoords.X - WTSR->DrawDistance;
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

			CLDR->addChunksToSpawnPosition(FChunkLocationData(ChunkPosition, newChunkCoords));
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
		if (PlayerChunkCoords.Y > InitialChunkCoords.Y) {
			// Get the first row by adding the draw distance to the player's Z
			lastRowZ = InitialChunkCoords.Y - WTSR->DrawDistance;
			newRowZ = InitialChunkCoords.Y + WTSR->DrawDistance;
		} else {
			// Get the last row by substracting the draw distance from the the initial Z
			lastRowZ = PlayerChunkCoords.Y + WTSR->DrawDistance;
			newRowZ = PlayerChunkCoords.Y - WTSR->DrawDistance;
		}

		const int firstIndexChunkX = PlayerChunkCoords.X - WTSR->DrawDistance + 1;
		const int lastIndexChunkX = PlayerChunkCoords.X + WTSR->DrawDistance + 1;

		// Loop and remove the entire row of chunks 
		for (int x = firstIndexChunkX; x < lastIndexChunkX; x++) {
			FIntPoint oldChunkCoords = FIntPoint(x, lastRowZ);
			CLDR->addChunksToDestroyPosition(oldChunkCoords);

			FIntPoint newChunkCoords = FIntPoint(x, newRowZ);
			FVector ChunkPosition = FVector(x * WTSR->chunkSize * WTSR->UnrealScale, newRowZ * WTSR->chunkSize * WTSR->UnrealScale, 0);

			CLDR->addChunksToSpawnPosition(FChunkLocationData(ChunkPosition, newChunkCoords));
		}
	}

	// Update the initial position for the next frame
	WTSR->updateInitialPlayerPosition(PlayerPosition);
}

FIntPoint TerrainRunnable::GetChunkCoordinates(FVector Position) const {
	const int32 ChunkX = FMath::FloorToInt(Position.X / (WTSR->chunkSize * WTSR->UnrealScale));
	const int32 ChunkY = FMath::FloorToInt(Position.Y / (WTSR->chunkSize * WTSR->UnrealScale));
	return FIntPoint(ChunkX, ChunkY);
}

void TerrainRunnable::SetWorldTerrainSettings(UWorldTerrainSettings* InWorldTerrainSettings) {
	WorldTerrainSettingsRef = InWorldTerrainSettings;
}

void TerrainRunnable::SetChunkLocationData(UChunkLocationData* InChunkLocationData) {
	ChunkLocationDataRef = InChunkLocationData;
}



