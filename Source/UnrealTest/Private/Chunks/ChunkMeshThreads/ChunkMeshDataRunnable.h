// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "..\..\Noise\PerlinNoiseSettings.h"
#include <vector>
#include <chrono>

#include "..\DataStructures\VoxelObjectMeshData.h"
#include "..\DataStructures\VoxelObjectLocationData.h"
#include "..\..\NPC\SettingsNPC\RelationshipSettingsNPC.h" // For the AnimalType enum
#include "CoreMinimal.h"

class FastNoiseLite;
class UWorldTerrainSettings;
class UChunkLocationData;

class ChunkMeshDataRunnable : public FRunnable {
	// Store solid blocks in a 3D chunk of chunkSize * chunkSize * intsPerHeight
	struct BinaryChunk3D {
		std::vector<uint64_t> yBinaryColumn;
		std::vector<uint64_t> xBinaryColumn;
		std::vector<uint64_t> zBinaryColumn;
	};

	enum class EDirection {
		Up, Down, Right, Left, Forward, Backward
	};

public:
	ChunkMeshDataRunnable(FVoxelObjectLocationData InChunkLocationData, UWorldTerrainSettings* InWorldTerrainSettingsRef, UChunkLocationData* InChunkLocationDataRef, APerlinNoiseSettings* InPerlinNoiseSettingsRef);
	virtual ~ChunkMeshDataRunnable() override;

	virtual bool Init() override;
	virtual uint32 Run() override;
	virtual void Stop() override;
	virtual void Exit() override;

	FThreadSafeBool IsTaskComplete() const;

	void SetChunkLocationData(UChunkLocationData* InChunkLocationData);
	void SetWorldTerrainSettings(UWorldTerrainSettings* InWorldTerrainSettings);
	void SetPerlinNoiseSettings(APerlinNoiseSettings* InPerlinNoiseSettingsRef);

private:
	UWorldTerrainSettings* WorldTerrainSettingsRef;
	UWorldTerrainSettings*& WTSR = WorldTerrainSettingsRef;

	UChunkLocationData* ChunkLocationDataRef;
	UChunkLocationData*& CLDR = ChunkLocationDataRef;

	APerlinNoiseSettings* PerlinNoiseSettingsRef;
	APerlinNoiseSettings*& PNSR = PerlinNoiseSettingsRef;

	FVoxelObjectLocationData ChunkLocationData;

	FThreadSafeBool isRunning;
	FThreadSafeBool isTaskComplete;

	// Create chrono type alias
	using Time = std::chrono::high_resolution_clock::time_point;

	BinaryChunk3D binaryChunk = BinaryChunk3D{};

	std::vector<uint64_t> columnsFaceMask;

	FVoxelObjectMeshData TemporaryMeshData; // store vertices, normals, triangles, etc.

	int vertexCount{ 0 };

	TArray<FVector2D> surfaceAvoidPositions;

	void createBinarySolidColumnsYXZ();

	void faceCullingBinaryColumnsYXZ(std::vector<std::vector<uint64_t>>& columnFaceMasks);

	void createTerrainMeshesData();

	void buildBinaryPlanes(const std::vector<uint64_t>& faceMaskColumn, std::vector<uint64_t>& binaryPlane, const int& axis);

	void greedyMeshingBinaryPlane(std::vector<uint64_t>& planes, const int& axis);

	void createAllVoxelPositionsFromOriginal(
		FVector& voxelPosition1,
		FVector& voxelPosition2,
		FVector& voxelPosition3,
		FVector& voxelPosition4,
		const int& width,
		const int& height,
		const int& axis);

	void createQuadAndAddToMeshData(
		const FVector& voxelPosition1,
		const FVector& voxelPosition2,
		const FVector& voxelPosition3,
		const FVector& voxelPosition4,
		const int& height, const int& width,
		const int& axis);

	int getColorIndexFromVoxelHeight(const int& height);

	void AddSpawnLocationForVegetationOrNpc(const int& x, const int& z, const int& height, const FVector& chunkWorldLocation);

	// Batching the spawn points to send them once to CLDR
	TArray<FVoxelObjectLocationData> PendingTreeSpawns;
	TArray<FVoxelObjectLocationData> PendingFlowerSpawns;
	TArray<FVoxelObjectLocationData> PendingGrassSpawns;
	TArray<TPair<FVoxelObjectLocationData, AnimalType>> PendingNpcSpawns;

	float GetMiddleOfVoxelObjectPosition(
		const int& location,
		const double& worldLocation,
		const uint8_t& objectSize,
		const uint8_t& objectScale,
		const uint8_t& objectHalfScale
	);

	// Add spawn points for vegetation and NPCs
	void AddGrassSpawnPoint(const int& x, const int& z, const int& height, const FVector& chunkWorldLocation, const int& colorIndex);
	void AddFlowerSpawnPoint(const int& x, const int& z, const int& height, const FVector& chunkWorldLocation, const int& colorIndex);
	void AddTreeSpawnPoint(const int& x, const int& z, const int& height, const FVector& chunkWorldLocation);
	void AddNpcSpawnPoint(const int& x, const int& z, const int& height, const FVector& chunkWorldLocation, const float& spawnObjectChance);


	// NPC Spawn chances
	AnimalType GetAnimalTypeFromSpawnChance();

	TArray<TPair<AnimalType, float>> AnimalSpawnChances = {
		{ AnimalType::RedPanda, 0.05f },
		{ AnimalType::Tapir,    0.15f },
		{ AnimalType::Sloth,    0.3f },
		{ AnimalType::Cobra,    0.4f },
		{ AnimalType::Bat,      0.5f },
		{ AnimalType::Peacock,  0.6f },
		{ AnimalType::Gorilla,  0.9f },
		{ AnimalType::Panda,    1.0f }
	};

};
