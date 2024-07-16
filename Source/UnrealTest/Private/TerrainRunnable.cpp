// Fill out your copyright notice in the Description page of Project Settings.

#include "WorldTerrainSettings.h"
#include "ChunkLocationData.h"
#include "TerrainRunnable.h"

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
		WorldTerrainSettingsRef->UpdateChunkCriticalSection.Lock();
		UpdateChunks();
		WorldTerrainSettingsRef->UpdateChunkCriticalSection.Unlock();

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
	FIntPoint InitialChunkCoords = GetChunkCoordinates(WorldTerrainSettingsRef->getInitialPlayerPosition());

	// Add and remove chunks on the X axis 
	if (PlayerChunkCoords.X != InitialChunkCoords.X) {

		int lastRowX{ 0 };
		int newRowX{ 0 };
		if (PlayerChunkCoords.X > InitialChunkCoords.X) {
			// Get the first row by adding the draw distance to the player's X
			lastRowX = PlayerChunkCoords.X - WorldTerrainSettingsRef->DrawDistance; // MAYBE -1 ; STILL TESTING
			newRowX = PlayerChunkCoords.X + WorldTerrainSettingsRef->DrawDistance;
		} else {
			// Get the last row by substracting the draw distance from the the initial X
			lastRowX = InitialChunkCoords.X + WorldTerrainSettingsRef->DrawDistance; // MAYBE -1 ; STILL TESTING
			newRowX = InitialChunkCoords.X - WorldTerrainSettingsRef->DrawDistance; // MAYBE -1 ; STILL TESTING
		}

		// Add new chunks and remove old chunks based on the player's new X position
		int firstIndexChunkZ = InitialChunkCoords.Y - WorldTerrainSettingsRef->DrawDistance;
		int lastIndexChunkZ = InitialChunkCoords.Y + WorldTerrainSettingsRef->DrawDistance;

		// Loop and remove the entire row of chunks 
		for (int z = firstIndexChunkZ; z < lastIndexChunkZ; z++) {
			FIntPoint oldChunkCoords = FIntPoint(lastRowX, z);

			ChunkLocationDataRef->addChunksToDestroyPosition(oldChunkCoords);

			// Adding new row 
			FIntPoint newChunkCoords = FIntPoint(newRowX, z);
			FVector ChunkPosition = FVector(newRowX * WorldTerrainSettingsRef->chunkSize * WorldTerrainSettingsRef->UnrealScale, z * WorldTerrainSettingsRef->chunkSize * WorldTerrainSettingsRef->UnrealScale, 0);

			ChunkLocationDataRef->addChunksToSpawnPosition(FChunkLocationData(ChunkPosition, newChunkCoords));
		}
	}

	// Update the initial position for the next frame
	PlayerChunkCoords = GetChunkCoordinates(PlayerPosition);
	InitialChunkCoords = GetChunkCoordinates(WorldTerrainSettingsRef->getInitialPlayerPosition());

	// Add and remove chunks on the Y axis 
	if (PlayerChunkCoords.Y != InitialChunkCoords.Y) {
		// Add new chunks and remove old chunks based on the player's new Y position
		int lastRowZ{ 0 };
		int newRowZ{ 0 };
		if (PlayerChunkCoords.Y > InitialChunkCoords.Y) {
			// Get the first row by adding the draw distance to the player's Z
			lastRowZ = InitialChunkCoords.Y - WorldTerrainSettingsRef->DrawDistance;
			newRowZ = InitialChunkCoords.Y + WorldTerrainSettingsRef->DrawDistance;
		} else {
			// Get the last row by substracting the draw distance from the the initial Z
			lastRowZ = PlayerChunkCoords.Y + WorldTerrainSettingsRef->DrawDistance;
			newRowZ = PlayerChunkCoords.Y - WorldTerrainSettingsRef->DrawDistance;
		}

		int firstIndexChunkX = PlayerChunkCoords.X - WorldTerrainSettingsRef->DrawDistance + 1;
		int lastIndexChunkX = PlayerChunkCoords.X + WorldTerrainSettingsRef->DrawDistance + 1;

		// Loop and remove the entire row of chunks 
		for (int x = firstIndexChunkX; x < lastIndexChunkX; x++) {
			FIntPoint oldChunkCoords = FIntPoint(x, lastRowZ);
			ChunkLocationDataRef->addChunksToDestroyPosition(oldChunkCoords);

			FIntPoint newChunkCoords = FIntPoint(x, newRowZ);
			FVector ChunkPosition = FVector(x * WorldTerrainSettingsRef->chunkSize * WorldTerrainSettingsRef->UnrealScale, newRowZ * WorldTerrainSettingsRef->chunkSize * WorldTerrainSettingsRef->UnrealScale, 0);

			ChunkLocationDataRef->addChunksToSpawnPosition(FChunkLocationData(ChunkPosition, newChunkCoords));
		}
	}

	// Update the initial position for the next frame
	WorldTerrainSettingsRef->updateInitialPlayerPosition(PlayerPosition);
}

FIntPoint TerrainRunnable::GetChunkCoordinates(FVector Position) const {
	int32 ChunkX = FMath::FloorToInt(Position.X / (WorldTerrainSettingsRef->chunkSize * WorldTerrainSettingsRef->UnrealScale));
	int32 ChunkY = FMath::FloorToInt(Position.Y / (WorldTerrainSettingsRef->chunkSize * WorldTerrainSettingsRef->UnrealScale));
	return FIntPoint(ChunkX, ChunkY);
}


void TerrainRunnable::SetWorldTerrainSettings(UWorldTerrainSettings* InWorldTerrainSettings) {
	WorldTerrainSettingsRef = InWorldTerrainSettings;
}

void TerrainRunnable::SetChunkLocationData(UChunkLocationData* InChunkLocationData) {
	ChunkLocationDataRef = InChunkLocationData;
}



