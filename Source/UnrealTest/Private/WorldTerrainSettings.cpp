// Fill out your copyright notice in the Description page of Project Settings.


#include "WorldTerrainSettings.h"

UWorldTerrainSettings::UWorldTerrainSettings() {
	// Reserve memory for double the draw distance for X and Z 
	SpawnedChunksMap.Reserve(DrawDistance * DrawDistance * 2); 

	// Initialize the critical sections locks


}

void UWorldTerrainSettings::AddChunkToMap(const FIntPoint& ChunkCoordinates, AActor* ChunkActor) {
	FScopeLock ScopeLock(&MapCriticalSection);
	SpawnedChunksMap.Add(ChunkCoordinates, ChunkActor);
}

AActor* UWorldTerrainSettings::GetAndRemoveChunkFromMap(const FIntPoint& ChunkCoordinates) {
	FScopeLock ScopeLock(&MapCriticalSection);
	return SpawnedChunksMap.FindAndRemoveChecked(ChunkCoordinates);
}

int UWorldTerrainSettings::GetMapSize() {
	FScopeLock ScopeLock(&MapCriticalSection);
	return SpawnedChunksMap.Num();
}

void UWorldTerrainSettings::updateInitialPlayerPosition(FVector newPosition) {
	FScopeLock ScopeLock(&PlayerPositionCriticalSection);
	playerInitialPosition = newPosition;
}

FVector UWorldTerrainSettings::getInitialPlayerPosition() {
	FScopeLock ScopeLock(&PlayerPositionCriticalSection);
	return playerInitialPosition;
}

void UWorldTerrainSettings::EmptyChunkMap() {
	FScopeLock ScopeLock(&MapCriticalSection);
	SpawnedChunksMap.Empty();
}

void UWorldTerrainSettings::printMapElements(FString message) {
	FScopeLock ScopeLock(&MapCriticalSection);
	int testCounter{ 0 };
	FString keysString = message + ": \n";

	for (const TPair<FIntPoint, AActor*>& Pair : SpawnedChunksMap) {

		if (!Pair.Value) {
			UE_LOG(LogTemp, Error, TEXT("Null actor found in SpawnedChunksMap at coordinates X=%d, Z=%d"), Pair.Key.X, Pair.Key.Y);
			continue;
		}

		keysString += FString::Printf(TEXT("\tX=%d, Z=%d "), Pair.Key.X, Pair.Key.Y);
		testCounter++;

		if (testCounter == DrawDistance * 2) {
			keysString += TEXT("\n");
			testCounter = 0;
		}
	}
	UE_LOG(LogTemp, Warning, TEXT("%s"), *keysString);
}

