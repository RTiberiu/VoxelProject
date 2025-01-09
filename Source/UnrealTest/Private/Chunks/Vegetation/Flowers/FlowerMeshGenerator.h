#pragma once

#include "..\..\DataStructures\VoxelObjectMeshData.h"
#include "..\..\DataStructures\VoxelObjectLocationData.h"
#include "CoreMinimal.h"

#include <vector>
#include <chrono>
#include <random>

#include "FlowerMeshGenerator.generated.h"

class UProceduralMeshComponent;
class UWorldTerrainSettings;

UCLASS()
class  UFlowerMeshGenerator : public UObject {
    GENERATED_BODY()

	struct BinaryFlower3D {
		std::vector<uint16_t> yBinaryColumn;
		std::vector<uint16_t> xBinaryColumn;
		std::vector<uint16_t> zBinaryColumn;
	};

public:
    UFlowerMeshGenerator();

    ~UFlowerMeshGenerator();

	void SetWorldTerrainSettings(UWorldTerrainSettings* InWorldTerrainSettings);

	FVoxelObjectMeshData GetFlowerMeshData();

private:
	UWorldTerrainSettings* WorldTerrainSettingsRef;
	UWorldTerrainSettings*& WTSR = WorldTerrainSettingsRef;

	FVoxelObjectLocationData FlowerLocationData;

	BinaryFlower3D binaryFlower = BinaryFlower3D{};

	std::vector<uint16_t> columnsFaceMask;

	FVoxelObjectMeshData MeshData; // store vertices, normals, triangles, etc.

	TArray<FVector> branchEndPoints;

	FColor stemColor;
	FColor petalsColor;

	int vertexCount{ 0 };

	void createTrunkBinarySolidColumnsYXZ();

	void createCrownBinarySolidColumnsYXZ();

	void GenerateSpherePoints(TArray<FVector>& CrownPoints, const int& branchX, const int& branchY, const int& branchZ);

	void AddTrunkPoints(TArray<FVector>& Points, float FlowerLength, float LastLayerProbability, int MaxBaseThickness);

	void printBinary(uint16_t value, int groupSize, const std::string& otherData);

	void createTerrainMeshesData(bool forFlowerTrunk);

	void faceCullingBinaryColumnsYXZ(std::vector<std::vector<uint16_t>>& columnFaceMasks);

	void buildBinaryPlanes(const std::vector<uint16_t>& faceMaskColumn, std::vector<uint16_t>& binaryPlane, const int& axis);

	void greedyMeshingBinaryPlane(std::vector<uint16_t>& planes, const int& axis, bool forFlowerTrunk);

	void createAllVoxelPositionsFromOriginal(
		FVector& voxelPosition1,
		FVector& voxelPosition2,
		FVector& voxelPosition3,
		FVector& voxelPosition4,
		const int& width,
		const int& height,
		const int& axis, bool forFlowerTrunk);

	void createQuadAndAddToMeshData(
		FVector& voxelPosition1,
		FVector& voxelPosition2,
		FVector& voxelPosition3,
		FVector& voxelPosition4,
		const int& height, const int& width,
		const int& axis, bool forFlowerTrunk);
};

