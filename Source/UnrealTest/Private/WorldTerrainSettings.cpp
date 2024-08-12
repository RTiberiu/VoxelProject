// Fill out your copyright notice in the Description page of Project Settings.

#include "WorldTerrainSettings.h"
#include "FairSemaphore.h"


UWorldTerrainSettings::UWorldTerrainSettings() : 
	UpdateChunkSemaphore(new FairSemaphore(1)),
	TickSemaphore(new FairSemaphore(1)),
	PlayerPositionSemaphore(new FairSemaphore(1)),
	MapSemaphore(new FairSemaphore(1)),
	DrawDistanceSemaphore(new FairSemaphore(1)) {
	// Reserve memory for double the draw distance for X and Z 
	SpawnedChunksMap.Reserve(DrawDistance * DrawDistance * 2); 
}

UWorldTerrainSettings::~UWorldTerrainSettings() {
	// Clean up semaphores
	delete UpdateChunkSemaphore;
	delete TickSemaphore;
	delete PlayerPositionSemaphore;
	delete MapSemaphore;
	delete DrawDistanceSemaphore;
}

void UWorldTerrainSettings::AddChunkToMap(const FIntPoint& ChunkCoordinates, AActor* ChunkActor) {
	MapSemaphore->Acquire();

	// Print the given parameters first
	if (ChunkActor) {
		UE_LOG(LogTemp, Log, TEXT("Adding Position: (X:%d, Z:%d), Adding Actor Address: %p, Adding Actor Name: %s"),
			ChunkCoordinates.X, ChunkCoordinates.Y, ChunkActor, *ChunkActor->GetName());
	} else {
		UE_LOG(LogTemp, Warning, TEXT("Adding Position: (X:%d, Z:%d), Adding Actor is nullptr"),
			ChunkCoordinates.X, ChunkCoordinates.Y);
	}

	printMapElements("Adding to map. ");

	SpawnedChunksMap.Add(ChunkCoordinates, ChunkActor);
	MapSemaphore->Release();
}

AActor* UWorldTerrainSettings::GetAndRemoveChunkFromMap(const FIntPoint& ChunkCoordinates) {
	MapSemaphore->Acquire();
	AActor* RemovedChunk = nullptr;

	UE_LOG(LogTemp, Log, TEXT("Removing position: (X:%d, Z:%d)"), ChunkCoordinates.X, ChunkCoordinates.Y);
	printMapElements("Removing from map. ");

	if (SpawnedChunksMap.Contains(ChunkCoordinates)) {
		RemovedChunk =  SpawnedChunksMap.FindAndRemoveChecked(ChunkCoordinates);
	} else {
		// throw std::runtime_error("Chunk coordinates not found in SpawnedChunksMap");
		UE_LOG(LogTemp, Error, TEXT("Chunk coordinates not found in SpawnedChunksMap: X=%d, Y=%d"), ChunkCoordinates.X, ChunkCoordinates.Y);
	}
	MapSemaphore->Release();
	return RemovedChunk;
	// return SpawnedChunksMap.FindAndRemoveChecked(ChunkCoordinates);
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

void UWorldTerrainSettings::printMapElements(FString message) {
	int testCounter{ 0 };
	FString keysString = message + ": \n";

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

