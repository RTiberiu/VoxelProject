// Fill out your copyright notice in the Description page of Project Settings.


#include "TerrainRunnable.h"

TerrainRunnable::TerrainRunnable(UWorld* World, TSubclassOf<AActor>* Chunk) : World(World), Chunk(Chunk), isRunning(false), isTaskComplete(false) {
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

	UpdateGameThreadEvent->Reset();
	AsyncTask(ENamedThreads::GameThread, [this]() {
		PlayerPosition = World->GetFirstPlayerController()->GetPawn()->GetActorLocation();
		UpdateGameThreadEvent->Trigger();
	});

	// Wait for the game thread to update the world
	UpdateGameThreadEvent->Wait();

	FIntPoint PlayerChunkCoords = GetChunkCoordinates(PlayerPosition);
	FIntPoint InitialChunkCoords = GetChunkCoordinates(getInitialPlayerPosition());

	// Add and remove chunks on the X axis 
	if (PlayerChunkCoords.X != InitialChunkCoords.X) {

		int lastRowX{ 0 };
		int newRowX{ 0 };
		if (PlayerChunkCoords.X > InitialChunkCoords.X) {
			UE_LOG(LogTemp, Warning, TEXT("MINUS drawsitance for lastRowX"));
			// Get the first row by adding the draw distance to the player's X
			lastRowX = PlayerChunkCoords.X - DrawDistance.load(); // MAYBE -1 ; STILL TESTING
			newRowX = PlayerChunkCoords.X + DrawDistance.load();
		} else {
			// Get the last row by substracting the draw distance from the the initial X
			lastRowX = InitialChunkCoords.X + DrawDistance.load(); // MAYBE -1 ; STILL TESTING
			newRowX = InitialChunkCoords.X - DrawDistance.load(); // MAYBE -1 ; STILL TESTING
			UE_LOG(LogTemp, Warning, TEXT("PLUS drawsitance for lastRowX"));
		}

		// Add new chunks and remove old chunks based on the player's new X position
		UE_LOG(LogTemp, Warning, TEXT("Player moved on X axis to chunk: X=%d"), PlayerChunkCoords.X);

		// Print SpawnedChunksMap keys before destroying
		printMapElements("SpawnedChunksMap Keys BEFORE X update");

		int firstIndexChunkZ = InitialChunkCoords.Y - DrawDistance.load();
		int lastIndexChunkZ = InitialChunkCoords.Y + DrawDistance.load();

		UE_LOG(LogTemp, Warning, TEXT("Attempting to get: X=%d - Z=%d to Z=%d"), lastRowX, firstIndexChunkZ, lastIndexChunkZ);
		UE_LOG(LogTemp, Warning, TEXT("InitialChunkCoords: X=%d Z=%d "), InitialChunkCoords.X, InitialChunkCoords.Y);
		UE_LOG(LogTemp, Warning, TEXT("PlayerChunkCoords: X=%d Z=%d "), PlayerChunkCoords.X, PlayerChunkCoords.Y);
		UE_LOG(LogTemp, Warning, TEXT("Adding to row X=%d"), newRowX);

		// Loop and remove the entire row of chunks 
		for (int z = firstIndexChunkZ; z < lastIndexChunkZ; z++) {
			UE_LOG(LogTemp, Warning, TEXT("Getting from map X=%d Z=%d"), lastRowX, z);
			AActor* currentChunk = GetAndRemoveChunkFromMap(FIntPoint(lastRowX, z));

			UpdateGameThreadEvent->Reset();
			AsyncTask(ENamedThreads::GameThread, [currentChunk]() {
				currentChunk->Destroy();
				UpdateGameThreadEvent->Trigger();
			});

			// Wait for the game thread to update the world
			UpdateGameThreadEvent->Wait();

			// currentChunk->Destroy();
			UE_LOG(LogTemp, Warning, TEXT("Got the coordinates and destroyed object"));

			// Adding new row 
			FIntPoint newChunkCoords = FIntPoint(newRowX, z);
			FVector ChunkPosition = FVector(newRowX * chunkSize * UnrealScale, z * chunkSize * UnrealScale, 0);
			/*AActor* SpawnedChunk = World->SpawnActor<AActor>(*Chunk, ChunkPosition, FRotator::ZeroRotator);
			if (SpawnedChunk) {
				UE_LOG(LogTemp, Warning, TEXT("Adding to map X=%d Z=%d"), newRowX, z);
				AddChunkToMap(newChunkCoords, SpawnedChunk);
				UE_LOG(LogTemp, Warning, TEXT("Added to map new coordinates and chunk."));
			}*/

			UpdateGameThreadEvent->Reset();
			AsyncTask(ENamedThreads::GameThread, [this, ChunkPosition, newChunkCoords]() {
				FIntPoint testChunkCoord = GetChunkCoordinates(ChunkPosition);
				// UE_LOG(LogTemp, Warning, TEXT("ATTEMPTING to spawn chunk at coordinates X=%d Z=%d"), testChunkCoord.X, testChunkCoord.Y);
				AActor* SpawnedChunk = World->SpawnActor<AActor>(*Chunk, ChunkPosition, FRotator::ZeroRotator);
				if (SpawnedChunk) {
					// UE_LOG(LogTemp, Warning, TEXT("Added to map chunk at coordinates X=%d Z=%d"), newChunkCoords.X, newChunkCoords.Y);
					AddChunkToMap(newChunkCoords, SpawnedChunk);
				}
				UpdateGameThreadEvent->Trigger();
			});

			// Wait for the game thread to update the world
			UpdateGameThreadEvent->Wait();

		}

		// Print SpawnedChunksMap keys after adding
		printMapElements("SpawnedChunksMap Keys AFTER X update");
	}

	// Update the initial position for the next frame
	//PlayerPositionCriticalSection.Lock();
	//playerInitialPosition = PlayerPosition;

	//// Testing position
	//FIntPoint playerPositionTest1 = GetChunkCoordinates(playerInitialPosition);
	//UE_LOG(LogTemp, Warning, TEXT("Change playerInitialPosition to X=%d Z=%d"), static_cast<int>(playerPositionTest1.X), static_cast<int>(playerPositionTest1.Y));
	//PlayerPositionCriticalSection.Unlock();

	// Update the initial position for the next frame
	PlayerChunkCoords = GetChunkCoordinates(PlayerPosition);
	InitialChunkCoords = GetChunkCoordinates(getInitialPlayerPosition());

	// Testing position
	FIntPoint playerPositionTest1 = GetChunkCoordinates(PlayerPosition);
	UE_LOG(LogTemp, Warning, TEXT("Inside Thread: Change playerInitialPosition BEFORE VALIDATING FOR Z CHANGE:  X=%d Z=%d"), playerPositionTest1.X, playerPositionTest1.Y);

	// Add and remove chunks on the Y axis 
	if (PlayerChunkCoords.Y != InitialChunkCoords.Y) {
		// Add new chunks and remove old chunks based on the player's new Y position
		UE_LOG(LogTemp, Warning, TEXT("Player moved on Z axis to chunk: Z=%d"), PlayerChunkCoords.Y);

		// Print SpawnedChunksMap keys before destroying
		printMapElements("SpawnedChunksMap Keys BEFORE Z update");

		int lastRowZ{ 0 };
		int newRowZ{ 0 };
		if (PlayerChunkCoords.Y > InitialChunkCoords.Y) {
			// Get the first row by adding the draw distance to the player's Z
			UE_LOG(LogTemp, Warning, TEXT("MINUS drawsitance for lastRowZ"));
			lastRowZ = InitialChunkCoords.Y - DrawDistance.load();
			newRowZ = InitialChunkCoords.Y + DrawDistance.load();
		} else {
			// Get the last row by substracting the draw distance from the the initial Z
			UE_LOG(LogTemp, Warning, TEXT("PLUS drawsitance for lastRowZ"));
			lastRowZ = PlayerChunkCoords.Y + DrawDistance.load();
			newRowZ = PlayerChunkCoords.Y - DrawDistance.load();
		}

		int firstIndexChunkX = PlayerChunkCoords.X - DrawDistance.load() + 1;
		int lastIndexChunkX = PlayerChunkCoords.X + DrawDistance.load() + 1;

		UE_LOG(LogTemp, Warning, TEXT("Attempting to get: X=%d to X=%d - Z=%d"), firstIndexChunkX, lastIndexChunkX, lastRowZ);
		UE_LOG(LogTemp, Warning, TEXT("InitialChunkCoords: X=%d Z=%d "), InitialChunkCoords.X, InitialChunkCoords.Y);
		UE_LOG(LogTemp, Warning, TEXT("PlayerChunkCoords: X=%d Z=%d "), PlayerChunkCoords.X, PlayerChunkCoords.Y);
		UE_LOG(LogTemp, Warning, TEXT("Adding to row Z=%d"), newRowZ);


		// Loop and remove the entire row of chunks 
		for (int x = firstIndexChunkX; x < lastIndexChunkX; x++) {
			FIntPoint oldChunkCoords = FIntPoint(x, lastRowZ);
			UE_LOG(LogTemp, Warning, TEXT("Getting from map X=%d Z=%d"), oldChunkCoords.X, oldChunkCoords.Y);
			AActor* currentChunk = GetAndRemoveChunkFromMap(oldChunkCoords);
			// currentChunk->Destroy();

			UpdateGameThreadEvent->Reset();
			AsyncTask(ENamedThreads::GameThread, [currentChunk]() {
				currentChunk->Destroy();
				UpdateGameThreadEvent->Trigger();
			});

			// Wait for the game thread to update the world
			UpdateGameThreadEvent->Wait();

			UE_LOG(LogTemp, Warning, TEXT("Got the coordinates and destroyed object"));

			FIntPoint newChunkCoords = FIntPoint(x, newRowZ);
			FVector ChunkPosition = FVector(x * chunkSize * UnrealScale, newRowZ * chunkSize * UnrealScale, 0);
			/*AActor* SpawnedChunk = World->SpawnActor<AActor>(*Chunk, ChunkPosition, FRotator::ZeroRotator);

			if (SpawnedChunk) {
				UE_LOG(LogTemp, Warning, TEXT("Adding to map X=%d Z=%d"), newChunkCoords.X, newChunkCoords.Y);
				AddChunkToMap(newChunkCoords, SpawnedChunk);
				UE_LOG(LogTemp, Warning, TEXT("Added the new coordinates"));
			}*/

			UpdateGameThreadEvent->Reset();
			AsyncTask(ENamedThreads::GameThread, [this, ChunkPosition, newChunkCoords]() {
				FIntPoint testChunkCoord = GetChunkCoordinates(ChunkPosition);
				// UE_LOG(LogTemp, Warning, TEXT("ATTEMPTING to spawn chunk at coordinates X=%d Z=%d"), testChunkCoord.X, testChunkCoord.Y);
				AActor* SpawnedChunk = World->SpawnActor<AActor>(*Chunk, ChunkPosition, FRotator::ZeroRotator);
				if (SpawnedChunk) {
					// UE_LOG(LogTemp, Warning, TEXT("Added to map chunk at coordinates X=%d Z=%d"), newChunkCoords.X, newChunkCoords.Y);
					AddChunkToMap(newChunkCoords, SpawnedChunk);
				}
				UpdateGameThreadEvent->Trigger();
			});

			// Wait for the game thread to update the world
			UpdateGameThreadEvent->Wait();
		}

		// Print SpawnedChunksMap keys after adding
		printMapElements("SpawnedChunksMap Keys AFTER Z update");
	}

	// Update the initial position for the next frame
	updateInitialPlayerPosition(PlayerPosition);

	// Testing position
	FIntPoint playerPositionTest = GetChunkCoordinates(PlayerPosition);
	UE_LOG(LogTemp, Warning, TEXT("Inside Thread: Change playerInitialPosition to X=%d Z=%d"), playerPositionTest.X, playerPositionTest.Y);
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



