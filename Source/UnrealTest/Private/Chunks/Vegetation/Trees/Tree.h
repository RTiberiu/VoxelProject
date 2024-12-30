#pragma once

#include "..\..\..\Noise\PerlinNoiseSettings.h"

//#include "..\..\ChunkData\ChunkLocationData.h" // This is here JUST for FChunkLocationData struct. TODO Move the struct in a different file
#include "..\..\DataStructures\VoxelObjectLocationData.h"

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
		std::vector<uint64_t> yBinaryColumn;
		std::vector<uint64_t> xBinaryColumn;
		std::vector<uint64_t> zBinaryColumn;
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

	std::vector<uint64_t> columnsFaceMask;

	FVoxelObjectMeshData TemporaryMeshData; // store vertices, normals, triangles, etc.

	TArray<FVector> branchEndPoints;

	FColor trunkColor;
	FColor crownColor;

	int vertexCount{ 0 };

	FIntPoint treeLocation;
	bool hasCollision;

	// Create chrono type alias
	using Time = std::chrono::high_resolution_clock::time_point;

	void printExecutionTime(Time& start, Time& end, const char* functionName);

	// TODO This might be combined or something. They are just the modified version from the ChunkMeshDataRunnable.cpp
	void createTrunkBinarySolidColumnsYXZ();

	void createCrownBinarySolidColumnsYXZ();

	void GenerateSpherePoints(TArray<FVector>& CrownPoints, const int& branchX, const int& branchY, const int& branchZ);

	void AddTrunkPoints(TArray<FVector>& Points, float TreeLength, float LastLayerProbability, int MaxBaseThickness);

	void printBinary(uint64_t value, int groupSize, const std::string& otherData);

	void createTerrainMeshesData(bool forTreeTrunk);

	void faceCullingBinaryColumnsYXZ(std::vector<std::vector<uint64_t>>& columnFaceMasks);

	void buildBinaryPlanes(const std::vector<uint64_t>& faceMaskColumn, std::vector<uint64_t>& binaryPlane, const int& axis);

	void greedyMeshingBinaryPlane(std::vector<uint64_t>& planes, const int& axis, bool forTreeTrunk);

	void createAllVoxelPositionsFromOriginal(
		FVector& voxelPosition1,
		FVector& voxelPosition2,
		FVector& voxelPosition3,
		FVector& voxelPosition4,
		const int& width,
		const int& height,
		const int& axis, bool forTreeTrunk);

	void createQuadAndAddToMeshData(
		FVector& voxelPosition1,
		FVector& voxelPosition2,
		FVector& voxelPosition3,
		FVector& voxelPosition4,
		const int& height, const int& width,
		const int& axis, bool forTreeTrunk);

	void spawnTreeMeshes();



protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
};
