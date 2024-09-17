// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "PerlinNoiseSettings.h"
#include <vector>
#include <chrono>
#include "ChunkMeshData.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BinaryChunk.generated.h"

class FastNoiseLite;
class UWorldTerrainSettings;
class UProceduralMeshComponent;
class APerlinNoiseSettings;

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

	~ABinaryChunk();

	void SetWorldTerrainSettings(UWorldTerrainSettings* InWorldTerrainSettings);
	void SetPerlinNoiseSettings(APerlinNoiseSettings* InPerlinNoiseSettings);


private: 
	APerlinNoiseSettings* PerlinNoiseSettingsRef;
	APerlinNoiseSettings*& PNSR = PerlinNoiseSettingsRef;

	UWorldTerrainSettings* WorldTerrainSettingsRef;
	UWorldTerrainSettings*& WTSR = WorldTerrainSettingsRef; // creating an alias for the world terrain settings ref

	// Create chrono type alias
	using Time = std::chrono::high_resolution_clock::time_point;

	TObjectPtr<UProceduralMeshComponent> Mesh;
	TObjectPtr<FastNoiseLite> noise;
	TObjectPtr<FastNoiseLite> domainWarp;

	TObjectPtr<FastNoiseLite> adjacentBiomeNoise;
	TObjectPtr<FastNoiseLite> adjacentBiomeDomainWarp;

	BinaryChunk3D binaryChunk = BinaryChunk3D{};

	std::vector<uint64_t> columnsFaceMask;

	FChunkMeshData MeshData; // store vertices, normals, triangles, etc.
	
	int vertexCount{ 0 };

	void createBinarySolidColumnsYXZ();

	void faceCullingBinaryColumnsYXZ(std::vector<std::vector<uint64_t>>& columnFaceMasks);

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

	void greedyMeshingBinaryPlane(std::vector<uint64_t>& planes, const int& axis);

	void createTerrainMeshesData();

	void buildBinaryPlanes(const std::vector<uint64_t>& faceMaskColumn, std::vector<uint64_t>& binaryPlane, const int& axis);

	void spawnTerrainChunkMeshes();

	void testingMeshingCreation();

	void printExecutionTime(Time& start, Time& end, const char* functionName);

	void printBinary(uint64_t value, int groupSize, const std::string& otherData ="Column value: ");

	void apply3DNoiseToHeightColumn(uint64_t& column, int& x, int& z, int& y, int& bitIndex, const FVector& chunkWorldLocation, int& height);

	int getBiomeIndexForCurrentLocation(const FVector& chunkWorldLocation);

	bool shouldChunkBeBlendedOnAxis(const FVector& chunkWorldLocation, const int& voxelLocation, const bool& axis);


	void setNoiseSettingsForBiome(const int& biomeIndex, const int& octaveIndex, const TObjectPtr<FastNoiseLite> noise, const TObjectPtr<FastNoiseLite> domainWarp);

	float getBiomeInterpolationWeightOnAxis(const FVector& worldLocation, const int& voxelLocation, const bool& axis);

	int getColorIndexFromVoxelHeight(const FVector& voxelPosition);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

};
