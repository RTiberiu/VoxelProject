// Fill out your copyright notice in the Description page of Project Settings.

#include "WorldTerrainSettings.h"
#include "FairSemaphore.h"


UWorldTerrainSettings::UWorldTerrainSettings() : 
	UpdateChunkSemaphore(new FairSemaphore(1)),
	PlayerPositionSemaphore(new FairSemaphore(1)),
	MapSemaphore(new FairSemaphore(1)),
	DrawDistanceSemaphore(new FairSemaphore(1)) {
	// Reserve memory for double the draw distance for X and Z 
	SpawnedChunksMap.Reserve(DrawDistance * DrawDistance * 2); 
}

UWorldTerrainSettings::~UWorldTerrainSettings() {
	// Clean up semaphores
	delete UpdateChunkSemaphore;
	delete PlayerPositionSemaphore;
	delete MapSemaphore;
	delete DrawDistanceSemaphore;
}

void UWorldTerrainSettings::AddChunkToMap(const FIntPoint& ChunkCoordinates, AActor* ChunkActor) {
	MapSemaphore->Acquire();
	SpawnedChunksMap.Add(ChunkCoordinates, ChunkActor);
	MapSemaphore->Release();
}

AActor* UWorldTerrainSettings::GetAndRemoveChunkFromMap(const FIntPoint& ChunkCoordinates) {
	MapSemaphore->Acquire();
	AActor* RemovedChunk = nullptr;
	if (SpawnedChunksMap.Contains(ChunkCoordinates)) {
		RemovedChunk = SpawnedChunksMap.FindAndRemoveChecked(ChunkCoordinates);
	}
	MapSemaphore->Release();
	return RemovedChunk;
}

AActor* UWorldTerrainSettings::GetNextChunkFromMap() {
	MapSemaphore->Acquire();
	AActor* RemovedChunk = nullptr;
	FIntPoint keyToRemove;

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
	MapSemaphore->Release();
	return RemovedChunk;
}

int UWorldTerrainSettings::GetMapSize() {
	MapSemaphore->Acquire();
	int MapSize = SpawnedChunksMap.Num();
	MapSemaphore->Release();
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

void UWorldTerrainSettings::EmptyChunkMap() {
	MapSemaphore->Acquire();
	SpawnedChunksMap.Empty();
	MapSemaphore->Release();
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

	CheckNumberOfElements();
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

void UWorldTerrainSettings::CheckNumberOfElements() {
	if (SpawnedChunksMap.Num() < 100) {
		printMapElements("CheckNumberOfElements(): ");
		UE_LOG(LogTemp, Error, TEXT("Map has less than 100 items!: Map size: %d"), SpawnedChunksMap.Num());
	} else if (SpawnedChunksMap.Num() > 101) {
		printMapElements("CheckNumberOfElements(): ");
		UE_LOG(LogTemp, Error, TEXT("Map has more than 100 items!: Map size: %d"), SpawnedChunksMap.Num());
	}
}
