// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <cstdint>
#include <condition_variable> 
#include <mutex> 
#include <atomic>

#include "..\..\Utils\Semaphore\FairSemaphore.h"
#include "CoreMinimal.h"
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

	TArray<FColor> TreeTrunkColorArray = {
		FColor(46, 35, 77), FColor(47, 36, 76), FColor(47, 38, 75), FColor(48, 39, 73), FColor(49, 40, 72), FColor(49, 41, 71), FColor(50, 42, 70), FColor(50, 43, 69)
	};

	TArray<FColor> TreeCrownColorArray = {
		FColor(151, 89, 142), FColor(156, 94, 147), FColor(160, 100, 151), FColor(163, 108, 154), FColor(165, 115, 158),
		FColor(185, 116, 150), FColor(187, 129, 158), FColor(190, 142, 166), FColor(193, 154, 173), FColor(197, 166, 181),
		FColor(221, 156, 156), FColor(224, 163, 163), FColor(228, 170, 170), FColor(231, 177, 177), FColor(233, 185, 185)
	};

	// Trees settings
	const uint8_t TreeVariations{ 10 };
	uint8_t TreeCount{ 0 };
	const uint8_t TreeScale{ 4 }; // this changes the voxel size (100 is 1m)
	const uint8_t TreeCountMax{ 1 };
	const uint16_t TreeHeight{ 248 }; // 5 bits
	const uint8_t TreeSize{ 62 }; 
	const uint8_t TreeSizePadding{ 64 }; 
	const uint8_t TreeIntsPerHeight{ static_cast<uint8_t>(TreeHeight / TreeSize) };
	const uint16_t MaxTreeTrunkWidth{ 8 };
	const uint16_t MinTreeTrunkWidth{ 4 };

	// Flowers settings
	const uint8_t FlowerVariations{ 10 };


	// Grass settings
	const uint8_t GrassVariations{ 10 };


	// Vegetation interacting methods 
	void AddTreeMeshData(FVoxelObjectMeshData treeData);
	void AddGrassMeshData(FVoxelObjectMeshData grassData);
	void AddFlowerMeshData(FVoxelObjectMeshData flowerData);

	FVoxelObjectMeshData GetRandomTreeMeshData();
	FVoxelObjectMeshData GetRandomGrassMeshData();
	FVoxelObjectMeshData GetRandomFlowerMeshData();

private:
	APerlinNoiseSettings* PerlinNoiseSettingsRef;
	APerlinNoiseSettings*& PNSR = PerlinNoiseSettingsRef;

	void CheckForDuplicateActorPointers();
	void CheckNumberOfElements();
	void CheckIfActorIsNullOrPendingKill();

	// Vegetation mesh data 
	TArray<FVoxelObjectMeshData> TreesMeshData;
	TArray<FVoxelObjectMeshData> GrassMeshData;
	TArray<FVoxelObjectMeshData> FlowersMeshData;

	// Player settings
	FVector playerInitialPosition;

	// Locks for critical sections
	FairSemaphore* PlayerPositionSemaphore;
	FairSemaphore* MapSemaphore;
	FairSemaphore* DrawDistanceSemaphore;

	// Map to store spawned chunks with 2D coordinates as keys
	TMap<FIntPoint, AActor*> SpawnedChunksMap;

	void initializePerlinNoise(TObjectPtr<FastNoiseLite>& noise);

	void applyPerlinNoiseSettings(TObjectPtr<FastNoiseLite>& noise, const int& settingsIndex);

	void initializeDomainWarpNoise(TObjectPtr<FastNoiseLite>& domainWarp);

	void applyDomainWarpSettings(TObjectPtr<FastNoiseLite>& domainWarp, const int& settingsIndex);

};

