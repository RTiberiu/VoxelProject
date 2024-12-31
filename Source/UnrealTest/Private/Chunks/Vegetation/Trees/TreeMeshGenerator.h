#pragma once

#include "..\..\DataStructures\VoxelObjectMeshData.h"
#include "..\..\DataStructures\VoxelObjectLocationData.h"
#include "CoreMinimal.h"

#include <vector>
#include <chrono>
#include <random>

#include "TreeMeshGenerator.generated.h"

class UProceduralMeshComponent;
class UWorldTerrainSettings;

UCLASS()
class  UTreeMeshGenerator : public UObject {
    GENERATED_BODY()

	struct BinaryTree3D {
		std::vector<uint32_t> yBinaryColumn;
		std::vector<uint32_t> xBinaryColumn;
		std::vector<uint32_t> zBinaryColumn;
	};

public:
    UTreeMeshGenerator();

    ~UTreeMeshGenerator();

	void SetWorldTerrainSettings(UWorldTerrainSettings* InWorldTerrainSettings);

	FVoxelObjectMeshData GetMeshTreeMeshData();

private:
	UWorldTerrainSettings* WorldTerrainSettingsRef;
	UWorldTerrainSettings*& WTSR = WorldTerrainSettingsRef;

	FVoxelObjectLocationData TreeLocationData;

	BinaryTree3D binaryTree = BinaryTree3D{};

	std::vector<uint32_t> columnsFaceMask;

	FVoxelObjectMeshData MeshData; // store vertices, normals, triangles, etc.

	TArray<FVector> branchEndPoints;

	FColor trunkColor;
	FColor crownColor;

	int vertexCount{ 0 };

	void createTrunkBinarySolidColumnsYXZ();

	void createCrownBinarySolidColumnsYXZ();

	void GenerateSpherePoints(TArray<FVector>& CrownPoints, const int& branchX, const int& branchY, const int& branchZ);

	void AddTrunkPoints(TArray<FVector>& Points, float TreeLength, float LastLayerProbability, int MaxBaseThickness);

	void printBinary(uint32_t value, int groupSize, const std::string& otherData);

	void createTerrainMeshesData(bool forTreeTrunk);

	void faceCullingBinaryColumnsYXZ(std::vector<std::vector<uint32_t>>& columnFaceMasks);

	void buildBinaryPlanes(const std::vector<uint32_t>& faceMaskColumn, std::vector<uint32_t>& binaryPlane, const int& axis);

	void greedyMeshingBinaryPlane(std::vector<uint32_t>& planes, const int& axis, bool forTreeTrunk);

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
};

