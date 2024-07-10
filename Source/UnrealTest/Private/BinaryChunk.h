// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <vector>
#include <chrono>
#include "WorldTerrainSettings.h"
#include "ChunkMeshData.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BinaryChunk.generated.h"

class FastNoiseLite;
class UProceduralMeshComponent;

UCLASS()
class ABinaryChunk : public AActor {
	GENERATED_BODY()

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
	// Sets default values for this actor's properties
	ABinaryChunk();

private: 
	// Create chrono type alias
	using Time = std::chrono::high_resolution_clock::time_point;

	TObjectPtr<UProceduralMeshComponent> Mesh;
	TObjectPtr<FastNoiseLite> noise;

	BinaryChunk3D binaryChunk = BinaryChunk3D{};

	std::vector<uint64_t> columnsFaceMask;

	FChunkMeshData MeshData; // store vertices, normals, triangles, etc.
	
	int vertexCount{ 0 };

	void createBinarySolidColumnsYXZ();

	void faceCullingBinaryColumnsYXZ(std::vector<std::vector<uint64_t>>& columnFaceMasks);

	FVector getVoxelStartingPosition(uint64_t& column, const int& axis, const int& x, const int& z, const int& bitIndex, const int& columnIndex);

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

	void greedyMeshingBinaryPlane();

	void createTerrainMeshesData();

	void spawnTerrainChunkMeshes();

	void testingMeshingCreation();

	void printExecutionTime(Time& start, Time& end, const char* functionName);

	void printBinary(uint64_t value, int groupSize);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

};
