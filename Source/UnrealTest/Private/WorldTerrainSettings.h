// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <cstdint>
#include <condition_variable> 
#include <mutex> 
#include <atomic>

#include "CoreMinimal.h"
#include "WorldTerrainSettings.generated.h"

UCLASS()
class UWorldTerrainSettings : public UObject {
	GENERATED_BODY()

public:
	UWorldTerrainSettings();

	int UnrealScale{ 100 };
	int DrawDistance{ 5 }; // TODO Maybe make atomic again. Removed to test if it fixes the loop in ChunkWorld line 53

	// Single chunk settings
	const uint16_t chunkHeight{ 248 }; // 4 bits 
	const uint8_t chunkSize{ 62 }; // 62
	const uint8_t chunkSizePadding{ 64 }; // 64
	const uint8_t intsPerHeight{ static_cast<uint8_t>(chunkHeight / chunkSize) };

	FEvent* UpdateGameThreadEvent;

	// Methods to interact with the world chunks
	void AddChunkToMap(const FIntPoint& ChunkCoordinates, AActor* ChunkActor);
	AActor* GetAndRemoveChunkFromMap(const FIntPoint& ChunkCoordinates);
	int GetMapSize();
	void updateInitialPlayerPosition(FVector newPosition);
	FVector getInitialPlayerPosition();
	void EmptyChunkMap();
	void printMapElements(FString message);

	FCriticalSection UpdateChunkCriticalSection;
	FCriticalSection TickCriticalSection;

private:
	// Player settings
	FVector playerInitialPosition;

	// Locks for critical sections
	FCriticalSection PlayerPositionCriticalSection;
	FCriticalSection MapCriticalSection;
	FCriticalSection DrawDistanceCriticalSection;

	// Map to store spawned chunks with 2D coordinates as keys
	TMap<FIntPoint, AActor*> SpawnedChunksMap;

};
