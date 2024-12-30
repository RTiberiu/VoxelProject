// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "..\..\Noise\PerlinNoiseSettings.h"
#include <vector>
#include <chrono>

#include "..\DataStructures\VoxelObjectMeshData.h"
#include "..\DataStructures\VoxelObjectLocationData.h"
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
	ChunkMeshDataRunnable(FVoxelObjectLocationData InChunkLocationData, UWorldTerrainSettings* InWorldTerrainSettingsRef, UChunkLocationData* InChunkLocationDataRef);
	virtual ~ChunkMeshDataRunnable() override;

	virtual bool Init() override;
	virtual uint32 Run() override;
	virtual void Stop() override;
	virtual void Exit() override;

	FThreadSafeBool IsTaskComplete() const;

	void SetChunkLocationData(UChunkLocationData* InChunkLocationData);
	void SetWorldTerrainSettings(UWorldTerrainSettings* InWorldTerrainSettings);

private:
	UWorldTerrainSettings* WorldTerrainSettingsRef;
	UWorldTerrainSettings*& WTSR = WorldTerrainSettingsRef;

	UChunkLocationData* ChunkLocationDataRef;
	UChunkLocationData*& CLDR = ChunkLocationDataRef;

	FVoxelObjectLocationData ChunkLocationData;

	FThreadSafeBool isRunning;
	FThreadSafeBool isTaskComplete;

	// Create chrono type alias
	using Time = std::chrono::high_resolution_clock::time_point;

	BinaryChunk3D binaryChunk = BinaryChunk3D{};

	std::vector<uint64_t> columnsFaceMask;

	FVoxelObjectMeshData TemporaryMeshData; // store vertices, normals, triangles, etc.

	int vertexCount{ 0 };

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

	int getColorIndexFromVoxelHeight(const FVector& voxelPosition);


};
