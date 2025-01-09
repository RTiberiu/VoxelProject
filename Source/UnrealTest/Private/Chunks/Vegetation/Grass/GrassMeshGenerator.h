#pragma once

#include "..\..\DataStructures\VoxelObjectMeshData.h"
#include "..\..\DataStructures\VoxelObjectLocationData.h"
#include "CoreMinimal.h"

#include <vector>
#include <chrono>
#include <random>

#include "GrassMeshGenerator.generated.h"

class UProceduralMeshComponent;
class UWorldTerrainSettings;

UCLASS()
class  UGrassMeshGenerator : public UObject {
    GENERATED_BODY()

	struct BinaryGrass3D {
		std::vector<uint8_t> yBinaryColumn;
		std::vector<uint8_t> xBinaryColumn;
		std::vector<uint8_t> zBinaryColumn;
	};

public:
    UGrassMeshGenerator();

    ~UGrassMeshGenerator();

	void SetWorldTerrainSettings(UWorldTerrainSettings* InWorldTerrainSettings);

	FVoxelObjectMeshData GetGrassMeshData();

private:
	UWorldTerrainSettings* WorldTerrainSettingsRef;
	UWorldTerrainSettings*& WTSR = WorldTerrainSettingsRef;

	FVoxelObjectLocationData GrassLocationData;

	BinaryGrass3D binaryGrass = BinaryGrass3D{};

	std::vector<uint8_t> columnsFaceMask;

	FVoxelObjectMeshData MeshData; // store vertices, normals, triangles, etc.

	TArray<FVector> branchEndPoints;

	FColor BladeColor;

	int vertexCount{ 0 };

	void createBladeBinarySolidColumnsYXZ();

	void createCrownBinarySolidColumnsYXZ();

	void createTerrainMeshesData();

	void faceCullingBinaryColumnsYXZ(std::vector<std::vector<uint8_t>>& columnFaceMasks);

	void buildBinaryPlanes(const std::vector<uint8_t>& faceMaskColumn, std::vector<uint8_t>& binaryPlane, const int& axis);

	void greedyMeshingBinaryPlane(std::vector<uint8_t>& planes, const int& axis);

	void createAllVoxelPositionsFromOriginal(
		FVector& voxelPosition1,
		FVector& voxelPosition2,
		FVector& voxelPosition3,
		FVector& voxelPosition4,
		const int& width,
		const int& height,
		const int& axis);

	void createQuadAndAddToMeshData(
		FVector& voxelPosition1,
		FVector& voxelPosition2,
		FVector& voxelPosition3,
		FVector& voxelPosition4,
		const int& height, const int& width,
		const int& axis);
};

