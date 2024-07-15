// Fill out your copyright notice in the Description page of Project Settings.

#include "TerrainRunnable.h"

TerrainRunnable::TerrainRunnable(FVector PlayerPosition) : PlayerPosition(PlayerPosition), isRunning(false), isTaskComplete(false) {
	UpdateGameThreadEvent = FPlatformProcess::GetSynchEventFromPool(true);
}

TerrainRunnable::~TerrainRunnable() {
	FPlatformProcess::ReturnSynchEventToPool(UpdateGameThreadEvent);
	UpdateGameThreadEvent = nullptr;
}

bool TerrainRunnable::Init() {
	isRunning = true;
	isTaskComplete = false;
	return true;
}

uint32 TerrainRunnable::Run() {
	while (isRunning) {

		// Spawn new chunks
		UpdateChunkCriticalSection.Lock();
		UpdateChunks();
		UpdateChunkCriticalSection.Unlock();

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

	// FUCKED UP EVERYTHING.
	// GOT TO MAKE SURE THE POSITIONS ARE CORRECT, AS I THINK THE THREADS ARE FUCKING UP
	// HOW THEY RETRIEVE THE PLAYERPOSITION AND THE PLAYERINITIALPOSITION.

	// THE ONLY FUCKED UP ONE I THINK IS THE PLAYERPOSITION FOR NOW. GOT TO MAKE SURE 
	// THAT IT IS UPDATING CORRECTLY. 

	FIntPoint PlayerChunkCoords = GetChunkCoordinates(PlayerPosition);
	FIntPoint InitialChunkCoords = GetChunkCoordinates(getInitialPlayerPosition());

	// Add and remove chunks on the X axis 
	if (PlayerChunkCoords.X != InitialChunkCoords.X) {

		int lastRowX{ 0 };
		int newRowX{ 0 };
		if (PlayerChunkCoords.X > InitialChunkCoords.X) {
			// Get the first row by adding the draw distance to the player's X
			lastRowX = PlayerChunkCoords.X - DrawDistance.load(); // MAYBE -1 ; STILL TESTING
			newRowX = PlayerChunkCoords.X + DrawDistance.load();
		} else {
			// Get the last row by substracting the draw distance from the the initial X
			lastRowX = InitialChunkCoords.X + DrawDistance.load(); // MAYBE -1 ; STILL TESTING
			newRowX = InitialChunkCoords.X - DrawDistance.load(); // MAYBE -1 ; STILL TESTING
		}

		// Add new chunks and remove old chunks based on the player's new X position
		int firstIndexChunkZ = InitialChunkCoords.Y - DrawDistance.load();
		int lastIndexChunkZ = InitialChunkCoords.Y + DrawDistance.load();

		// Loop and remove the entire row of chunks 
		for (int z = firstIndexChunkZ; z < lastIndexChunkZ; z++) {
			FIntPoint oldChunkCoords = FIntPoint(lastRowX, z);

			UE_LOG(LogTemp, Warning, TEXT("X -- Added coords to DESTROY: X=%d Z=%d"), oldChunkCoords.X, oldChunkCoords.Y);
			UChunkLocationData::getInstance().addChunksToDestroyPosition(oldChunkCoords);

			/*FIntPoint chunkToDestroyPosition;
			bool isDestroyPositionReturned = getChunkToDestroyPosition(chunkToDestroyPosition);

			UE_LOG(LogTemp, Warning, TEXT("[TerrainRunnable] isDestroyPositionReturned: %s, ChunkToDestroyPosition: X=%d, Y=%d"),
				isDestroyPositionReturned ? TEXT("true") : TEXT("false"),
				chunkToDestroyPosition.X,
				chunkToDestroyPosition.Y);*/


			// Adding new row 
			FIntPoint newChunkCoords = FIntPoint(newRowX, z);
			FVector ChunkPosition = FVector(newRowX * chunkSize * UnrealScale, z * chunkSize * UnrealScale, 0);

			UE_LOG(LogTemp, Warning, TEXT("X -- Added coords to SPAWN: X=%d Z=%d"), newChunkCoords.X, newChunkCoords.Y);
			UChunkLocationData::getInstance().addChunksToSpawnPosition(FChunkLocationData(ChunkPosition, newChunkCoords));

			
			/*FChunkLocation chunkToSpawnPosition;
			bool isSpawnPositionReturned = getChunkToSpawnPosition(chunkToSpawnPosition);

			UE_LOG(LogTemp, Warning, TEXT("[TerrainRunnable] isSpawnPositionReturned: %s, ChunkToSpawnPosition: ChunkPosition=(%f, %f, %f), ChunkWorldCoords: X=%d, Y=%d"),
				isSpawnPositionReturned ? TEXT("true") : TEXT("false"),
				chunkToSpawnPosition.ChunkPosition.X,
				chunkToSpawnPosition.ChunkPosition.Y,
				chunkToSpawnPosition.ChunkPosition.Z,
				chunkToSpawnPosition.ChunkWorldCoords.X,
				chunkToSpawnPosition.ChunkWorldCoords.Y);*/
		}
	}

	// Update the initial position for the next frame
	PlayerChunkCoords = GetChunkCoordinates(PlayerPosition);
	InitialChunkCoords = GetChunkCoordinates(getInitialPlayerPosition());

	// Add and remove chunks on the Y axis 
	if (PlayerChunkCoords.Y != InitialChunkCoords.Y) {
		// Add new chunks and remove old chunks based on the player's new Y position
		int lastRowZ{ 0 };
		int newRowZ{ 0 };
		if (PlayerChunkCoords.Y > InitialChunkCoords.Y) {
			// Get the first row by adding the draw distance to the player's Z
			lastRowZ = InitialChunkCoords.Y - DrawDistance.load();
			newRowZ = InitialChunkCoords.Y + DrawDistance.load();
		} else {
			// Get the last row by substracting the draw distance from the the initial Z
			lastRowZ = PlayerChunkCoords.Y + DrawDistance.load();
			newRowZ = PlayerChunkCoords.Y - DrawDistance.load();
		}

		int firstIndexChunkX = PlayerChunkCoords.X - DrawDistance.load() + 1;
		int lastIndexChunkX = PlayerChunkCoords.X + DrawDistance.load() + 1;

		// Loop and remove the entire row of chunks 
		for (int x = firstIndexChunkX; x < lastIndexChunkX; x++) {
			FIntPoint oldChunkCoords = FIntPoint(x, lastRowZ);

			UE_LOG(LogTemp, Warning, TEXT("Z -- Added coords to DESTROY: X=%d Z=%d"), oldChunkCoords.X, oldChunkCoords.Y);
			UChunkLocationData::getInstance().addChunksToDestroyPosition(oldChunkCoords);


			FIntPoint newChunkCoords = FIntPoint(x, newRowZ);
			FVector ChunkPosition = FVector(x * chunkSize * UnrealScale, newRowZ * chunkSize * UnrealScale, 0);

			UE_LOG(LogTemp, Warning, TEXT("Z -- Added coords to SPAWN: X=%d Z=%d"), newChunkCoords.X, newChunkCoords.Y);
			UChunkLocationData::getInstance().addChunksToSpawnPosition(FChunkLocationData(ChunkPosition, newChunkCoords));

		}
	}

	// Update the initial position for the next frame
	updateInitialPlayerPosition(PlayerPosition);
}

FIntPoint TerrainRunnable::GetChunkCoordinates(FVector Position) const {
	int32 ChunkX = FMath::FloorToInt(Position.X / (chunkSize * UnrealScale));
	int32 ChunkY = FMath::FloorToInt(Position.Y / (chunkSize * UnrealScale));
	return FIntPoint(ChunkX, ChunkY);
}

void TerrainRunnable::AddChunkToMap(const FIntPoint& ChunkCoordinates, AActor* ChunkActor) {
	FScopeLock ScopeLock(&MapCriticalSection);
	SpawnedChunksMap.Add(ChunkCoordinates, ChunkActor);
}

AActor* TerrainRunnable::GetAndRemoveChunkFromMap(const FIntPoint& ChunkCoordinates) {
	FScopeLock ScopeLock(&MapCriticalSection);
	return SpawnedChunksMap.FindAndRemoveChecked(ChunkCoordinates);
}

int TerrainRunnable::GetMapSize() {
	FScopeLock ScopeLock(&MapCriticalSection);
	return SpawnedChunksMap.Num();
}

void TerrainRunnable::updateInitialPlayerPosition(FVector newPosition) {
	FScopeLock ScopeLock(&PlayerPositionCriticalSection);
	playerInitialPosition = newPosition;
}

FVector TerrainRunnable::getInitialPlayerPosition() {
	FScopeLock ScopeLock(&PlayerPositionCriticalSection);
	return playerInitialPosition;
}

void TerrainRunnable::EmptyChunkMap() {
	FScopeLock ScopeLock(&MapCriticalSection);
	SpawnedChunksMap.Empty();
}

void TerrainRunnable::printMapElements(FString message) {
	FScopeLock ScopeLock(&MapCriticalSection);
	int testCounter{ 0 };
	FString keysString = message + ": \n";

	for (const TPair<FIntPoint, AActor*>& Pair : SpawnedChunksMap) {
		keysString += FString::Printf(TEXT("\tX=%d, Z=%d "), Pair.Key.X, Pair.Key.Y);
		testCounter++;

		if (testCounter == DrawDistance.load() * 2) {
			keysString += TEXT("\n");
			testCounter = 0;
		}
	}
	UE_LOG(LogTemp, Warning, TEXT("%s"), *keysString);
}



