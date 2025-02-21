// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "..\Vegetation\Trees\Tree.h"
#include "..\Vegetation\Trees\TreeMeshGenerator.h"
#include "..\Vegetation\Grass\GrassMeshGenerator.h"
#include "..\Vegetation\Flowers\FlowerMeshGenerator.h"
#include "..\SingleChunk\BinaryChunk.h"
#include "..\ChunkMeshThreads\ChunksLocationRunnable.h"
#include "..\ChunkMeshThreads\ChunkMeshDataRunnable.h"
#include "..\ChunkData\ChunkLocationData.h"

#include "..\..\NPC\SettingsNPC\AnimationSettingsNPC.h"

#include "Pathfinding/PathfindingThreadPool/PathfindingThreadManager.h"

#include "..\..\Utils\CustomMesh\CustomProceduralMeshComponent.h"

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
	void SetAnimationSettingsNpc(UAnimationSettingsNPC* InAnimationSettingsRef);

	void InitializePathfindingManager();

	PathfindingThreadManager* PathfindingManager;
private:
	APerlinNoiseSettings* PerlinNoiseSettingsRef;
	APerlinNoiseSettings*& PNSR = PerlinNoiseSettingsRef;

	UWorldTerrainSettings* WorldTerrainSettingsRef;
	UWorldTerrainSettings*& WTSR = WorldTerrainSettingsRef; // creating an alias for the world terrain settings ref

	UChunkLocationData* ChunkLocationDataRef;
	UChunkLocationData*& CLDR = ChunkLocationDataRef; // creating an alias for the chunk location data ref

	UAnimationSettingsNPC* AnimationSettingsRef;
	UAnimationSettingsNPC*& AnimS = AnimationSettingsRef; // creating an alias for the animation settings ref

	TSubclassOf<AActor> Chunk;

	// Create chrono type alias // TODO This is a duplicate from BinaryChunk.h; Add this in a UTIL header
	using Time = std::chrono::high_resolution_clock::time_point;

	void printExecutionTime(Time& start, Time& end, const char* functionName); // TODO This is a duplicate from BinaryChunk.h; Add this in a UTIL header

	// Ensure that the Tick() function runs after the BeginPlay() is done initializing the world
	FThreadSafeBool isInitialWorldGenerated;

	void spawnInitialWorld();

	// Generating all the vegetation meshes variations and adding them to a cached list
	void generateTreeMeshVariations();
	void generateGrassMeshVariations();
	void generateFlowerMeshVariations();

	// Runnable to handle spawning the chunks
	ChunksLocationRunnable* chunksLocationRunnable;
	FRunnableThread* chunksLocationThread;
	FThreadSafeBool isLocationTaskRunning;

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

	// Tree implementation
	void SpawnTrees(FVoxelObjectLocationData ChunkLocationData, FVector PlayerPosition);
	TSubclassOf<AActor> Tree;

	void SpawnGrass(FVoxelObjectLocationData ChunkLocationData, FVector PlayerPosition);
	void SpawnFlower(FVoxelObjectLocationData ChunkLocationData, FVector PlayerPosition);
	void SpawnNPC(FVoxelObjectLocationData ChunkLocationData, FVector PlayerPosition);

	// NPC Settings
	TSubclassOf<AActor> NPC;
	
	// Helper methods to remove vegetation spawn points and destroy actors
	void RemoveVegetationSpawnPointsAndActors(const FIntPoint& destroyPosition);
	void DestroyTreeActors();
	void DestroyGrassActors();
	void DestroyFlowerActors();
	void DestroyNpcActors();

	// Tree actors to be destroyed and settings
	TArray<ATree*> TreeActorsToRemove;
	const int treesToRemovePerFrame = 1;

	TArray<FVoxelObjectLocationData> TreePositionsToSpawn;
	const int treesToSpawnPerFrame = 1;

	// Grass actors to be destroyed and settings
	TArray<UCustomProceduralMeshComponent*> GrassActorsToRemove;
	const int grassToRemovePerFrame = 3;

	TArray<FVoxelObjectLocationData> GrassPositionsToSpawn;
	const int grassToSpawnPerFrame = 3;

	// Flower actors to be destroyed and settings
	TArray<UCustomProceduralMeshComponent*> FlowerActorsToRemove;
	const int flowerToRemovePerFrame = 3;

	TArray<FVoxelObjectLocationData> FlowerPositionsToSpawn;
	const int flowerToSpawnPerFrame = 3;

	// NPC actors to be destroyed and settings
	TArray<FVoxelObjectLocationData> NPCPositionsToSpawn;
	const int npcToSpawnPerFrame = 2;

	TArray<ABasicNPC*> NpcActorsToRemove;
	const int npcToRemovePerFrame = 2;

	bool spawnedTreesThisFrame;
	bool spawnedChunksThisFrame;

	int updatePlayerCurrentPositionCounter = 0;
	int updatePlayerCurrentPositionPerFrames = 60;
	void updatePlayerCurrentPosition(FVector& PlayerPosition);

	// Control variable for printing the chunk mesh compute time 
	int lastLoggedChunkCount = 0;



protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called when the game ends or when destroyed
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	FIntPoint GetChunkCoordinates(FVector Position) const;

	virtual void Tick(float DeltaSeconds) override;

};
