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
#include <Chunks/Vegetation/Grass/Grass.h>
#include <Chunks/Vegetation/Flowers/Flower.h>
#include "WorldTerrainSettings.generated.h"

class FastNoiseLite;
class APerlinNoiseSettings;

UCLASS()
class UWorldTerrainSettings : public UObject {
	GENERATED_BODY()

public:
	UWorldTerrainSettings();

	~UWorldTerrainSettings();

	const uint8_t UnrealScale{ 50 }; // this changes the voxel size (100 is 1m)
	const uint8_t DrawDistance{ 5 }; // 5 

	// Single chunk settings
	const uint16_t chunkHeight{ 248 }; // 4 bits
	const uint8_t chunkSize{ 62 }; // 62
	const uint8_t chunkSizePadding{ 64 }; // 64
	const uint8_t intsPerHeight{ static_cast<uint8_t>(chunkHeight / chunkSize) };

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
	void updateInitialPlayerPosition(FVector newPosition);
	FVector getInitialPlayerPosition();
	void EmptyChunkMap();
	const TMap<FIntPoint, AActor*>& GetSpawnedChunksMap() const;

	void UpdateChunksCollision(FVector& PlayerPosition);

	void UpdateTreeCollisions(FVector& PlayerPosition);

	FairSemaphore* UpdateChunkSemaphore;

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
	const int GrassColorEndIndex = 9;
	const int LayerHeight = 10;
	const int ColorLayers = 20;

	TArray<FColor> TreeTrunkColorArray = {
		FColor(46, 35, 77), FColor(47, 36, 76), FColor(47, 38, 75), FColor(48, 39, 73), FColor(49, 40, 72), FColor(49, 41, 71), FColor(50, 42, 70), FColor(50, 43, 69)
	};

	TArray<FColor> TreeCrownColorArray = {
		FColor(151, 89, 142), FColor(156, 94, 147), FColor(160, 100, 151), FColor(163, 108, 154), FColor(165, 115, 158),
		FColor(185, 116, 150), FColor(187, 129, 158), FColor(190, 142, 166), FColor(193, 154, 173), FColor(197, 166, 181),
		FColor(221, 156, 156), FColor(224, 163, 163), FColor(228, 170, 170), FColor(231, 177, 177), FColor(233, 185, 185)
	};

	// Vegetation settings
	const int VegetationCollisionDistance{ chunkSize * UnrealScale };
	const float TreeSpawnChance{ 0.002f };
	const float FlowerSpawnChance{ 0.04f };
	const float GrassSpawnChance{ 0.08f };

	// Trees settings
	const uint8_t TreeVariations{ 30 };
	const int TreeSpawnRadius{ DrawDistance / 2};
	const int TreeChunkRadius{ UnrealScale * TreeSpawnRadius * chunkSizePadding };
	int TreeCount{ 0 };
	const uint8_t TreeScale{ 15 }; // this changes the voxel size (100 is 1m)
	const uint8_t TreeCountMax{ 40 };
	const uint16_t TreeHeight{ 60 }; // 5 bits
	const uint8_t TreeSize{ 30 }; 
	const uint8_t TreeSizePadding{ 32 }; 
	const uint8_t TreeIntsPerHeight{ static_cast<uint8_t>(TreeHeight / TreeSize) };
	const uint16_t MaxTreeTrunkWidth{ 2 };
	const uint16_t MinTreeTrunkWidth{ 1 };

	// Flowers settings
	int FlowerCount{ 0 };
	const uint8_t FlowerVariations{ 30 };
	const uint8_t FlowerScale{ 15 }; // this changes the voxel size (100 is 1m)
	const uint8_t FlowerSizePadding{ 16 };
	const uint8_t FlowerSize{ 14 };
	const uint16_t FlowerHeight{ 14 }; // 1 bits
	const uint8_t FlowerIntsPerHeight{ static_cast<uint8_t>(FlowerHeight / FlowerSize) };

	TArray<FColor> FlowerColorArray = {
		FColor(3, 9, 28), FColor(8, 15, 79), FColor(17, 13, 130), FColor(45, 18, 181), FColor(86, 23, 232), FColor(145, 74, 237), FColor(191, 125, 242), FColor(226, 176, 247)
	};

	// Grass settings
	int GrassCount{ 0 };
	const uint8_t GrassVariations{ 30 };
	const uint8_t GrassScale{ 7 }; // this changes the voxel size (100 is 1m)
	const uint8_t GrassSizePadding{ 8 };
	const uint8_t GrassSize{ 6 };
	const uint16_t GrassHeight{ 12 }; // 2 bits
	const uint8_t GrassIntsPerHeight{ static_cast<uint8_t>(GrassHeight / GrassSize) };

	TArray<FColor> GrassBladesColorArray = {
		FColor(3, 9, 28), FColor(8, 15, 79), FColor(17, 13, 130), FColor(45, 18, 181), FColor(86, 23, 232), FColor(145, 74, 237), FColor(191, 125, 242), FColor(226, 176, 247)
	};



	// TODO I might want to move this to VegetationMeshData.cpp eventually
	// Vegetation interacting methods 
	void AddTreeMeshData(FVoxelObjectMeshData treeData);
	void AddGrassMeshData(FVoxelObjectMeshData grassData);
	void AddFlowerMeshData(FVoxelObjectMeshData flowerData);

	FVoxelObjectMeshData* GetRandomTreeMeshData();
	FVoxelObjectMeshData* GetRandomGrassMeshData();
	FVoxelObjectMeshData* GetRandomFlowerMeshData();

	void AddSpawnedTrees(const FIntPoint& TreeWorldCoordinates, ATree* TreeActor);
	void AddSpawnedGrass(const FIntPoint& GrassWorldCoordinates, AGrass* GrassActor);
	void AddSpawnedFlower(const FIntPoint& FlowerWorldCoordinates, AFlower* FlowerActor);
	const TMap<FIntPoint, TArray<ATree*>>& GetSpawnedTreesMap() const;
	TArray<ATree*> GetAndRemoveTreeFromMap(const FIntPoint& TreeWorldCoordinates);
	TArray<AGrass*> GetAndRemoveGrassFromMap(const FIntPoint& GrassWorldCoordinates);
	TArray<AFlower*> GetAndRemoveFlowerFromMap(const FIntPoint& FlowerWorldCoordinates);
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
	TMap<FIntPoint, TArray<AGrass*>> SpawnedGrassMap;
	TMap<FIntPoint, TArray<AFlower*>> SpawnedFlowerMap;

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

