#pragma once

#include "..\..\..\Noise\PerlinNoiseSettings.h"

#include "..\..\ChunkData\ChunkLocationData.h" // This is here JUST for FChunkLocationData struct. TODO Move the struct in a different file

// TODO This will be added in another object that stores the final tree meshes with all the variations
#include "..\..\ChunkData\VoxelObjectMeshData.h"

#include <vector>
#include <chrono>
#include <random>
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Tree.generated.h"


class UProceduralMeshComponent;
class APerlinNoiseSettings;
class UWorldTerrainSettings;


UCLASS()
class ATree : public AActor {
	GENERATED_BODY()

	// Store solid blocks in a 3D chunk of chunkSize * chunkSize * intsPerHeight
	// TODO This is also used in ChunkMeshDataRunnable, and I might just combine them instead of duplicating code
	struct BinaryTree3D {
		std::vector<uint32_t> yBinaryColumn;
		std::vector<uint32_t> xBinaryColumn;
		std::vector<uint32_t> zBinaryColumn;
	};

public:
	// Sets default values for this actor's properties
	ATree();
	~ATree();

	void SetTreeCollision(bool InHasCollision);

	void SetWorldTerrainSettings(UWorldTerrainSettings* InWorldTerrainSettings);
	void SetPerlinNoiseSettings(APerlinNoiseSettings* InPerlinNoiseSettings);

private:
	APerlinNoiseSettings* PerlinNoiseSettingsRef;
	APerlinNoiseSettings*& PNSR = PerlinNoiseSettingsRef;

	UWorldTerrainSettings* WorldTerrainSettingsRef;
	UWorldTerrainSettings*& WTSR = WorldTerrainSettingsRef;

	FVoxelObjectLocationData TreeLocationData;

	TObjectPtr<UProceduralMeshComponent> Mesh;

	BinaryTree3D binaryTree = BinaryTree3D{};

	std::vector<uint32_t> columnsFaceMask;

	FVoxelObjectMeshData TemporaryMeshData; // store vertices, normals, triangles, etc.

	int vertexCount{ 0 };

	FVoxelObjectMeshData meshData;
	FIntPoint treeLocation;
	bool hasCollision;

	// Create chrono type alias
	using Time = std::chrono::high_resolution_clock::time_point;

	void printExecutionTime(Time& start, Time& end, const char* functionName);

	// TODO This might be combined or something. They are just the modified version from the ChunkMeshDataRunnable.cpp
	void createBinarySolidColumnsYXZ();

	void apply3DNoiseToHeightColumn(uint32_t& column, int& x, int& z, int& y, int& bitIndex, const FVector& treeWorldLocation, int& height);

	void createTerrainMeshesData();

	void faceCullingBinaryColumnsYXZ(std::vector<std::vector<uint32_t>>& columnFaceMasks);

	void buildBinaryPlanes(const std::vector<uint32_t>& faceMaskColumn, std::vector<uint32_t>& binaryPlane, const int& axis);

	void greedyMeshingBinaryPlane(std::vector<uint32_t>& planes, const int& axis);

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

	void spawnTreeMeshes();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
};
