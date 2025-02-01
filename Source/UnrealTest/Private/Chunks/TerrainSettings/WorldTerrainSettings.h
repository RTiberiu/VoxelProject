// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <cstdint>
#include <condition_variable> 
#include <mutex> 
#include <atomic>

#include "..\..\Utils\Semaphore\FairSemaphore.h"
#include "..\DataStructures\VoxelObjectLocationData.h"
#include "..\DataStructures\VoxelObjectMeshData.h"
#include "CoreMinimal.h"
#include <Chunks/SingleChunk/BinaryChunk.h>
#include <Chunks/Vegetation/Trees/Tree.h>
#include "WorldTerrainSettings.generated.h"

class FastNoiseLite;
class APerlinNoiseSettings;

UCLASS()
class UWorldTerrainSettings : public UObject {
	GENERATED_BODY()

public:
	UWorldTerrainSettings();

	~UWorldTerrainSettings();

	const uint8_t UnrealScale{ 60 }; // this changes the voxel size (100 is 1m) // 50 
	const uint8_t DrawDistance{ 8 }; // 5 

	// Single chunk settings
	const uint16_t chunkHeight{ 248 }; // 4 bits
	const uint8_t chunkSize{ 62 }; // 62
	const uint8_t chunkSizePadding{ 64 }; // 64
	const uint8_t intsPerHeight{ static_cast<uint8_t>(chunkHeight / chunkSize) };

	std::chrono::duration<double, std::milli> chunkSpawnTime{ 0 };
	int chunksMeshCounter{0};

	// World chunks settings
	const int CollisionDistance{ chunkSize * UnrealScale * 2 };

	const int biomeWidth{ chunkSize * 5 }; // 10
	const uint64_t blendBiomeThreshold{ 150 };

	void ValidateSpawnedChunksMap();

	// Methods to interact with the world chunks
	void AddChunkToMap(const FIntPoint& ChunkCoordinates, AActor* ChunkActor);
	AActor* GetAndRemoveChunkFromMap(const FIntPoint& ChunkCoordinates);
	AActor* GetNextChunkFromMap();
	int GetMapSize();
	void EmptyChunkMap();
	const TMap<FIntPoint, AActor*>& GetSpawnedChunksMap() const;

	void UpdateChunksCollision(FVector& PlayerPosition);

	void UpdateTreeCollisions(FVector& PlayerPosition);

	FairSemaphore* UpdateChunkSemaphore;

	// Player positions (used to despawn objects and for pathfinding)
	void updateInitialPlayerPosition(FVector newPosition);
	FVector getInitialPlayerPosition();

	void updateCurrentPlayerPosition(FVector& newPosition);
	FVector& getCurrentPlayerPosition();


	// Chunk noises
	// Noise objects used to generate the world
	TObjectPtr<FastNoiseLite> continentalness;
	TObjectPtr<FastNoiseLite> erosion;
	TObjectPtr<FastNoiseLite> peaksAndValleys;

	// Domain warp for the 3 main noise objects
	TObjectPtr<FastNoiseLite> continentalnessDW;
	TObjectPtr<FastNoiseLite> erosionDW;
	TObjectPtr<FastNoiseLite> peaksAndValleysDW;

	void SetPerlinNoiseSettings(APerlinNoiseSettings* InPerlinNoiseSettings);

	void ApplyDomainWarpToCoords(float& noisePositionX, float& noisePositionZ, TObjectPtr<FastNoiseLite> noise);

	float GetNoiseAtCoords(float& noisePositionX, float& noisePositionZ, TObjectPtr<FastNoiseLite> noise);

	bool isActorPresentInMap(AActor* actor); // TODO REMOVE AFTER TESTING - USING THIS BEFORE DESTROY() 

	void printMapElements(FString message); // TODO SHOULD MADE PRIVATE AFTER TESTING

	TArray<FColor> ChunkColorArray = {
		FColor(45, 41, 32), FColor(48, 44, 35), FColor(79, 69, 55), FColor(105, 87, 59), FColor(143, 118, 80), FColor(113, 125, 73), FColor(109, 120, 67), FColor(98, 110, 59), FColor(88, 99, 49), FColor(78, 89, 39), FColor(53, 60, 29), FColor(41, 48, 25), FColor(36, 43, 20), FColor(63, 63, 63), FColor(73, 73, 73), FColor(89, 89, 89), FColor(99, 99, 94), FColor(140, 140, 140), FColor(230, 225, 210), FColor(255, 251, 233)
	};

	const int GrassColorStartIndex = 5;
	const int GrassColorEndIndex = 12;
	const int LayerHeight = 10;
	const int ColorLayers = 20;

	TArray<FColor> TreeTrunkColorArray = {
		FColor(54, 28, 23), FColor(58, 30, 24), FColor(61, 32, 26), FColor(65, 34, 27), FColor(68, 36, 29), FColor(72, 38, 30), FColor(76, 40, 32), FColor(79, 42, 33), FColor(83, 43, 35), FColor(86, 45, 36)
	};

	TArray<FColor> TreeCrownColorArray = {
		FColor(30, 72, 65), FColor(22, 80, 77), FColor(15, 81, 87), FColor(20, 89, 103), FColor(25, 95, 118), FColor(43, 102, 136), FColor(65, 107, 149), FColor(80, 100, 165), FColor(106, 104, 177), FColor(135, 101, 174), FColor(151, 103, 173), FColor(167, 103, 173)
	};

	// Vegetation settings
	const int VegetationCollisionDistance{ chunkSize * UnrealScale };
	const float TreeSpawnChance{ 0.0014f };
	const float FlowerSpawnChance{ 0.03f };	// Showcase settings: 0.03f		// Testing settings: 0.03f
	const float GrassSpawnChance{ 0.10f };	// Showcase settings: 0.08f		// Testing settings: 0.10f
	const float NPCSpawnChance{ 0.102f };	// Showcase settings: 0.082f	// Testing settings: 0.102f

	// Trees settings
	const uint8_t TreeVariations{ 30 };
	const int TreeSpawnRadius{ DrawDistance / 2};
	const int TreeChunkRadius{ UnrealScale * TreeSpawnRadius * chunkSizePadding };
	int TreeCount{ 0 };
	const uint8_t TreeScale{ 15 };
	const uint8_t TreeCountMax{ 40 };
	const uint16_t TreeHeight{ 60 }; // 5 bits
	const uint8_t TreeSize{ 30 }; 
	const uint8_t TreeSizePadding{ 32 }; 
	const uint8_t TreeIntsPerHeight{ static_cast<uint8_t>(TreeHeight / TreeSize) };
	const uint16_t MaxTreeTrunkWidth{ 2 };
	const uint16_t MinTreeTrunkWidth{ 1 };

	// Flowers settings
	int FlowerCount{ 0 };
	const uint8_t FlowerVariations{ 40 };
	const uint8_t FlowerScale{ 7 };
	const uint8_t FlowerSizePadding{ 16 };
	const uint8_t FlowerSize{ 14 };
	const int MaxFlowerSphere{ 2 };
	const uint16_t FlowerHeight{ 14 }; // 1 bits
	const uint8_t FlowerIntsPerHeight{ static_cast<uint8_t>(FlowerHeight / FlowerSize) };

	TArray<FColor> FlowerStemColorArray = {
		FColor(98, 117, 67), FColor(84, 108, 61), FColor(71, 99, 54), FColor(58, 90, 48), FColor(46, 80, 42)
	};

	TArray<FColor> FlowerPetalColorArray = {
		FColor(74, 86, 125), FColor(74, 80, 130), FColor(77, 75, 134),
		FColor(86, 76, 138), FColor(96, 77, 143), FColor(108, 77, 147),
		FColor(120, 78, 151), FColor(133, 79, 156), FColor(147, 79, 161),
		FColor(162, 80, 165), FColor(170, 80, 161), FColor(175, 80, 153),
		FColor(178, 83, 143), FColor(180, 85, 133), FColor(183, 87, 122),
		FColor(186, 90, 112), FColor(188, 92, 102), FColor(191, 96, 95),
		FColor(194, 112, 100)
	};

	// Grass settings
	int GrassCount{ 0 };
	const uint8_t GrassVariations{ 40 };
	const uint8_t GrassScale{ 7 };
	const uint8_t GrassSizePadding{ 16 };
	const uint8_t GrassSize{ 14 };
	const uint16_t GrassHeight{ 14 }; // 2 bits
	const uint8_t GrassIntsPerHeight{ static_cast<uint8_t>(GrassHeight / GrassSize) };

	TArray<FColor> GrassBladesColorArray = { FColor(98, 117, 67), FColor(84, 108, 61), FColor(71, 99, 54), FColor(58, 90, 48), FColor(46, 80, 42) };

	int NPCCount{ 0 };

	// TODO I might want to move this to VegetationMeshData.cpp eventually
	// Vegetation interacting methods 
	void AddTreeMeshData(FVoxelObjectMeshData treeData);
	void AddGrassMeshData(FVoxelObjectMeshData grassData);
	void AddFlowerMeshData(FVoxelObjectMeshData flowerData);

	FVoxelObjectMeshData* GetRandomTreeMeshData();
	FVoxelObjectMeshData* GetRandomGrassMeshData();
	FVoxelObjectMeshData* GetRandomFlowerMeshData();

	void AddSpawnedTrees(const FIntPoint& TreeWorldCoordinates, ATree* TreeActor);
	void AddSpawnedGrass(const FIntPoint& GrassWorldCoordinates, UProceduralMeshComponent* GrassActor);
	void AddSpawnedFlower(const FIntPoint& FlowerWorldCoordinates, UProceduralMeshComponent* FlowerActor);
	const TMap<FIntPoint, TArray<ATree*>>& GetSpawnedTreesMap() const;
	TArray<ATree*> GetAndRemoveTreeFromMap(const FIntPoint& TreeWorldCoordinates);
	TArray<UProceduralMeshComponent*> GetAndRemoveGrassFromMap(const FIntPoint& GrassWorldCoordinates);
	TArray<UProceduralMeshComponent*> GetAndRemoveFlowerFromMap(const FIntPoint& FlowerWorldCoordinates);
	void RemoveTreeFromMap(const FIntPoint& TreeWorldCoordinates);

	void AddChunkToRemoveCollision(ABinaryChunk* actor);
	void AddTreeToRemoveCollision(ATree* actor);
	void AddChunkToEnableCollision(ABinaryChunk* actor);
	void AddTreeToEnableCollision(ATree* actor);
	ABinaryChunk* GetChunkToRemoveCollision();
	ATree* GetTreeToRemoveCollision();
	ABinaryChunk* GetChunkToEnableCollision();
	ATree* GetTreeToEnableCollision();

private:
	APerlinNoiseSettings* PerlinNoiseSettingsRef;
	APerlinNoiseSettings*& PNSR = PerlinNoiseSettingsRef;

	void CheckForDuplicateActorPointers();
	void CheckNumberOfElements();
	void CheckIfActorIsNullOrPendingKill();
	void CheckForDuplicateWorldCoordinates();

	// Vegetation mesh data 
	TArray<FVoxelObjectMeshData> TreesMeshData;
	TArray<FVoxelObjectMeshData> GrassMeshData;
	TArray<FVoxelObjectMeshData> FlowersMeshData;

	// Player settings
	FVector playerInitialPosition;
	FVector playerCurrentPosition; // This is used for pathfinding and updated in ChunkWorld

	// Locks for critical sections
	FairSemaphore* PlayerPositionSemaphore;
	FairSemaphore* ChunkMapSemaphore;
	FairSemaphore* TreeMapSemaphore;
	FairSemaphore* GrassMapSemaphore;
	FairSemaphore* FlowerMapSemaphore;
	FairSemaphore* DrawDistanceSemaphore;
	FairSemaphore* AddCollisionTreesSemaphore;
	FairSemaphore* RemoveCollisionTreesSemaphore;
	FairSemaphore* AddCollisionChunksSemaphore;
	FairSemaphore* RemoveCollisionChunksSemaphore;

	// Map to store spawned chunks with 2D coordinates as keys
	TMap<FIntPoint, AActor*> SpawnedChunksMap;

	// Array to store spawned trees, grass, and flowers
	TMap<FIntPoint, TArray<ATree*>> SpawnedTreesMap;
	TMap<FIntPoint, TArray<UProceduralMeshComponent*>> SpawnedGrassMap;
	TMap<FIntPoint, TArray<UProceduralMeshComponent*>> SpawnedFlowerMap;

	// Arrays to update collision for actors (chunks and trees)
	TArray<ABinaryChunk*> RemoveCollisionChunks;
	TArray<ATree*> RemoveCollisionTrees;
	TArray<ABinaryChunk*> AddCollisionChunks;
	TArray<ATree*> AddCollisionTrees;

	void initializePerlinNoise(TObjectPtr<FastNoiseLite>& noise);

	void applyPerlinNoiseSettings(TObjectPtr<FastNoiseLite>& noise, const int& settingsIndex);

	void initializeDomainWarpNoise(TObjectPtr<FastNoiseLite>& domainWarp);

	void applyDomainWarpSettings(TObjectPtr<FastNoiseLite>& domainWarp, const int& settingsIndex);

};

