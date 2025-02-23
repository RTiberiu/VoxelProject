#pragma once

#include "..\..\Chunks\DataStructures\VoxelObjectMeshData.h"
#include "..\..\Chunks\DataStructures\VoxelObjectLocationData.h"
#include <vector>
#include "CoreMinimal.h"

UCLASS()
class UStatsMeshGenerator : public UObject {
	GENERATED_BODY()

	struct BinaryStats3D {
		std::vector<uint16_t> yBinaryColumn;
		std::vector<uint16_t> xBinaryColumn;
		std::vector<uint16_t> zBinaryColumn;
	};

public:

	UStatsMeshGenerator();
	~UStatsMeshGenerator();

	FVoxelObjectMeshData GetStatsMeshData(const FColor& InStatsColor, const int& InFilledVoxels);

private:
	FVoxelObjectLocationData StatsLocationData;
	BinaryStats3D binaryStats = BinaryStats3D{};

	std::vector<uint16_t> columnsFaceMask;
	FVoxelObjectMeshData MeshData;
	int vertexCount{ 0 };

	FColor StatsColor;
	int FilledVoxels { 0 };

	// Stats voxel settings
	const uint8_t StatsScale{ 7 };
	const float HalfStatsScale{ static_cast<float>(StatsScale) / 2.0f };
	const uint8_t StatsSizePadding{ 16 };
	const uint8_t StatsSize{ 14 };
	const uint16_t StatsHeight{ 14 };
	const uint8_t StatsIntsPerHeight{ static_cast<uint8_t>(StatsHeight / StatsSize) };

	const uint8_t StatsVoxelDimensions{ 3 };
	const uint8_t VoxelCubeStart{ 3 };


	void createStatsBinarySolidColumnsYXZ();

	TArray<FVector> getUniqueStatsPoints();

	void createTerrainMeshesData();

	void faceCullingBinaryColumnsYXZ(std::vector<std::vector<uint16_t>>& columnFaceMasks);

	void buildBinaryPlanes(const std::vector<uint16_t>& faceMaskColumn, std::vector<uint16_t>& binaryPlane, const int& axis);

	void greedyMeshingBinaryPlane(std::vector<uint16_t>& planes, const int& axis);

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
