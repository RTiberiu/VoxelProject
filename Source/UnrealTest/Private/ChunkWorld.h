// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

// #include "BinaryChunk.h"
#include "TerrainRunnable.h"
#include "ChunkMeshDataRunnable.h"
#include "ChunkLocationData.h"
#include <chrono>
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "ChunkWorld.generated.h"


class ABinaryChunk;
class WorldTerrainSettings; // forward declaration to the world settings
class UChunkLocationData;
class APerlinNoiseSettings;

UCLASS()
class AChunkWorld : public AActor {
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AChunkWorld();

	void SetWorldTerrainSettings(UWorldTerrainSettings* InWorldTerrainSettings);
	void SetChunkLocationData(UChunkLocationData* InChunkLocationData);
	void SetPerlinNoiseSettings(APerlinNoiseSettings* InPerlinNoiseSettings);

private:
	APerlinNoiseSettings* PerlinNoiseSettingsRef;
	APerlinNoiseSettings*& PNSR = PerlinNoiseSettingsRef;

	UWorldTerrainSettings* WorldTerrainSettingsRef;
	UWorldTerrainSettings*& WTSR = WorldTerrainSettingsRef; // creating an alias for the world terrain settings ref

	UChunkLocationData* ChunkLocationDataRef;
	UChunkLocationData*& CLDR = ChunkLocationDataRef; // creating an alias for the chunk location data ref

	TSubclassOf<AActor> Chunk;

	// Create chrono type alias // TODO This is a duplicate from BinaryChunk.h; Add this in a UTIL header
	using Time = std::chrono::high_resolution_clock::time_point;

	void printExecutionTime(Time& start, Time& end, const char* functionName); // TODO This is a duplicate from BinaryChunk.h; Add this in a UTIL header

	// Ensure that the Tick() function runs after the BeginPlay() is done initializing the world
	FThreadSafeBool isInitialWorldGenerated;

	void spawnInitialWorld();

	// Runnable to handle spawning the chunks
	TerrainRunnable* terrainRunnable;
	FRunnableThread* terrainThread;
	FThreadSafeBool isTerrainTaskRunning;

	ChunkMeshDataRunnable* chunkMeshDataRunnable;
	FRunnableThread* chunkMeshDataThread;
	FThreadSafeBool isMeshTaskRunning;

	// Handle logic after the terrain is generated 
	void onNewTerrainGenerated();

	// Calculate average time spawn for BinaryChunk
	void calculateAverageChunkSpawnTime(const Time& startTime, const Time& endTime);
	int32 ChunksSpawnedCount = 0;
	const int32 ChunksToAverage = 500;
	double TotalTimeForChunks = 0.0;

	// Destroy entire world when perlin noise settings change
	void destroyCurrentWorldChunks();

	// Update collisions to chunks, making sure they have collision if they're around the player
	void UpdateChunkCollisions(const FVector& PlayerPosition);

	// Control how many frames chunks get their collision checked and updated
	int frameCounterCollision;
	int framesUntilCollisionCheck = 10;

	TArray<FChunkLocationData> testingThreadLocations; // TODO Delete this when done testing

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	FIntPoint GetChunkCoordinates(FVector Position) const;

	virtual void Tick(float DeltaSeconds) override;

};
