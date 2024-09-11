// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <cstdint>
#include <condition_variable> 
#include <mutex> 
#include <atomic>

#include "FairSemaphore.h"

#include "CoreMinimal.h"
#include "WorldTerrainSettings.generated.h"

UCLASS()
class UWorldTerrainSettings : public UObject {
	GENERATED_BODY()

public:
	UWorldTerrainSettings();

	~UWorldTerrainSettings();

	const uint8_t UnrealScale{ 20 }; // this changes the voxel size (100 is 1m)
	const uint8_t DrawDistance{ 5 }; // 5 


	// Single chunk settings
	const uint16_t chunkHeight{ 248 }; // 4 bits
	const uint8_t chunkSize{ 62 }; // 62
	const uint8_t chunkSizePadding{ 64 }; // 64
	const uint8_t intsPerHeight{ static_cast<uint8_t>(chunkHeight / chunkSize) };

	// World chunks settings
	const uint16_t biomeWidth{ chunkSize * 10 };
	const uint64_t blendBiomeThreshold{ 100 };

	void ValidateSpawnedChunksMap();

	// Methods to interact with the world chunks
	void AddChunkToMap(const FIntPoint& ChunkCoordinates, AActor* ChunkActor);
	AActor* GetAndRemoveChunkFromMap(const FIntPoint& ChunkCoordinates);
	AActor* GetNextChunkFromMap();
	int GetMapSize();
	void updateInitialPlayerPosition(FVector newPosition);
	FVector getInitialPlayerPosition();
	void EmptyChunkMap();

	FairSemaphore* UpdateChunkSemaphore;

	bool isActorPresentInMap(AActor* actor); // TODO REMOVE AFTER TESTING - USING THIS BEFORE DESTROY() 

	void printMapElements(FString message); // TODO SHOULD MADE PRIVATE AFTER TESTING

	TArray<FColor> ColorArray = {
		//FColor(110, 9, 130), // Fantasy
		//FColor(130, 9, 120),
		//FColor(130, 9, 90),
		FColor(9, 127, 39), // Green
		FColor(9, 127, 69),
		FColor(9, 127, 98)
	};

private:
	void CheckForDuplicateActorPointers();
	void CheckNumberOfElements();
	void CheckIfActorIsNullOrPendingKill();

	// Player settings
	FVector playerInitialPosition;

	// Locks for critical sections
	FairSemaphore* PlayerPositionSemaphore;
	FairSemaphore* MapSemaphore;
	FairSemaphore* DrawDistanceSemaphore;

	// Map to store spawned chunks with 2D coordinates as keys
	TMap<FIntPoint, AActor*> SpawnedChunksMap;
};
