#include "StatsMeshGenerator.h"
#include <iostream>
#include <array>
#include <bitset>
#include <limits>
#include <set>
#include <string>
#include <bit>

// TODO In the future, if time allows, all the mesh generators should be moved in a single class inside Utils.
// Some of them differ, using uint16_t or uint64_t, and the Flower object also uses some different logic, running
// the mesh generation twice (once for the stem and once for the flower head).

UStatsMeshGenerator::UStatsMeshGenerator() {
}

UStatsMeshGenerator::~UStatsMeshGenerator() {
}

FVoxelObjectMeshData UStatsMeshGenerator::GetStatsMeshData(const FColor& InStatsColor, const int& InFilledVoxels) {
    
	StatsColor = InStatsColor;
	FilledVoxels = InFilledVoxels;

	createStatsBinarySolidColumnsYXZ();
	createTerrainMeshesData();

	FVoxelObjectMeshData TemporaryMeshData = MeshData;

	// Resetting the data structures
	MeshData.Vertices.Empty();
	MeshData.Triangles.Empty();
	MeshData.Normals.Empty();
	MeshData.Colors.Empty();
	MeshData.UV0.Empty();
	binaryStats = BinaryStats3D{};
	binaryStats.yBinaryColumn.clear();
	binaryStats.xBinaryColumn.clear();
	binaryStats.zBinaryColumn.clear();
	columnsFaceMask.clear();
	StatsLocationData.ObjectPosition = FVector::ZeroVector;
	StatsLocationData.ObjectWorldCoords = FIntPoint(0, 0);
	vertexCount = 0;

	return TemporaryMeshData;
}


void UStatsMeshGenerator::createStatsBinarySolidColumnsYXZ() {
	const FVector StatsWorldLocation = StatsLocationData.ObjectPosition;

	// Set the Stats values to air for all 3 axis (Y, X, Z)
	const int StatsDimensions{ StatsSizePadding * StatsSizePadding * StatsIntsPerHeight };
	binaryStats.xBinaryColumn = std::vector<uint16_t>(StatsDimensions, 0);
	binaryStats.yBinaryColumn = std::vector<uint16_t>(StatsDimensions, 0);
	binaryStats.zBinaryColumn = std::vector<uint16_t>(StatsDimensions, 0);

	const int halfStatsChunk = StatsSize / 2;

	// Get the points based on the FillVoxels amount
	TArray<FVector> uniqueStatsPoints = getUniqueStatsPoints();

	for (int point = 0; point < uniqueStatsPoints.Num(); point++) {
		const int x = uniqueStatsPoints[point].X;
		const int z = uniqueStatsPoints[point].Y;
		const int height = uniqueStatsPoints[point].Z;
		const int localStatsChunkHeight = (height % StatsHeight);
		const int bitIndex = FMath::FloorToInt(height / (float)StatsSizePadding);

		// Get index of y 
		const int yIndex{ (x * StatsSizePadding * StatsIntsPerHeight) + (z * StatsIntsPerHeight) + bitIndex };

		// Add blocks height data (Y) to the current X and Z
		binaryStats.yBinaryColumn[yIndex] |= (1U << localStatsChunkHeight);

		const uint16_t currentYCol = binaryStats.yBinaryColumn[yIndex];

		// Next Y index (column) means the same X index (column), but a change in Y bit index
		const int xIndex{ (height * StatsSizePadding) + x };

		binaryStats.xBinaryColumn[xIndex] |= (1U << z);

		// Next Y index (column) means the next Z index (column), but the same Y bit index
		const int zIndex{ (height * StatsSizePadding) + z };

		// Assign to actual bit the change
		binaryStats.zBinaryColumn[zIndex] |= (1U << x);
	}
}

TArray<FVector> UStatsMeshGenerator::getUniqueStatsPoints() {
	TArray<FVector> uniqueStatsPoints;
	const int adjustedVoxelDimensionEnd = StatsVoxelDimensions + VoxelCubeStart;
	int voxelCounter = 0;

	for (int y = VoxelCubeStart; y < adjustedVoxelDimensionEnd; y++) {
		for (int x = VoxelCubeStart; x < adjustedVoxelDimensionEnd; x++) {
			for (int z = VoxelCubeStart; z < adjustedVoxelDimensionEnd; z++) {

				// Return list if reached the filled voxel count
				if (voxelCounter >= FilledVoxels) {
					return uniqueStatsPoints;
				}

				// Add voxel point to the set and create the cube shape
				uniqueStatsPoints.Add(FVector(x, z, y));

				voxelCounter++;
			}
		}
	}
}

void UStatsMeshGenerator::createTerrainMeshesData() {
	// Storing the face masks for the Y, X, Z axis
	// Size is doubled to contains both ascending and descending columns 
	std::vector<std::vector<uint16_t>> columnFaceMasks{
		std::vector<uint16_t>(StatsSizePadding * StatsSizePadding * StatsIntsPerHeight), // Y ascending
		std::vector<uint16_t>(StatsSizePadding * StatsSizePadding * StatsIntsPerHeight), // Y descending
		std::vector<uint16_t>(StatsSizePadding * StatsSizePadding * StatsIntsPerHeight), // X ascending
		std::vector<uint16_t>(StatsSizePadding * StatsSizePadding * StatsIntsPerHeight), // X descending
		std::vector<uint16_t>(StatsSizePadding * StatsSizePadding * StatsIntsPerHeight), // Z ascending
		std::vector<uint16_t>(StatsSizePadding * StatsSizePadding * StatsIntsPerHeight), // Z descending
	};

	// Face cull the binary columns on the 3 axis, ascending and descending
	faceCullingBinaryColumnsYXZ(columnFaceMasks);

	// Storing planes for all axis, ascending and descending
	std::vector<std::vector<uint16_t>> binaryPlanes{
		std::vector<uint16_t>(StatsSizePadding * StatsSizePadding * StatsIntsPerHeight), // Y ascending
		std::vector<uint16_t>(StatsSizePadding * StatsSizePadding * StatsIntsPerHeight), // Y descending
		std::vector<uint16_t>(StatsSizePadding * StatsSizePadding * StatsIntsPerHeight), // X ascending
		std::vector<uint16_t>(StatsSizePadding * StatsSizePadding * StatsIntsPerHeight), // X descending
		std::vector<uint16_t>(StatsSizePadding * StatsSizePadding * StatsIntsPerHeight), // Z ascending
		std::vector<uint16_t>(StatsSizePadding * StatsSizePadding * StatsIntsPerHeight), // Z descending
	};

	for (int axis = 0; axis < 6; axis++) { // Iterate all axis ascending and descending 
		// Create the binary plane for each axis
		buildBinaryPlanes(columnFaceMasks[axis], binaryPlanes[axis], axis);

		// Greedy mesh each plane and create planes
		greedyMeshingBinaryPlane(binaryPlanes[axis], axis);
	}
}

void UStatsMeshGenerator::faceCullingBinaryColumnsYXZ(std::vector<std::vector<uint16_t>>& columnFaceMasks) {
	// Face culling for all the 3 axis (Y, X, Z)
	for (int axis = 0; axis < 3; axis++) {
		for (int x = 0; x < StatsSizePadding; x++) {
			for (int z = 0; z < StatsSizePadding; z++) {
				for (int bitIndex = 0; bitIndex < StatsIntsPerHeight; bitIndex++) {
					const int columnIndex{ (x * StatsSizePadding * StatsIntsPerHeight) + (z * StatsIntsPerHeight) + bitIndex };

					uint16_t column = 0;
					switch (axis) {
					case 0:
						column = binaryStats.yBinaryColumn[columnIndex];
						break;
					case 1:
						column = binaryStats.xBinaryColumn[columnIndex];
						break;
					case 2:
						column = binaryStats.zBinaryColumn[columnIndex];
						break;
					}

					// If is the Y axis and not the last bitIndex
					if (axis == 0 && bitIndex < StatsIntsPerHeight - 1) {
						const bool isAboveSolid = binaryStats.yBinaryColumn[columnIndex + 1] != 0;
						const bool columnAllSolid = column == std::numeric_limits<uint16_t>::max();

						// Skip creating face between height chunks if there's more solid blocks above 
						if (isAboveSolid && columnAllSolid) {
							continue;
						}
					}

					// Sample ascending axis and set to true when air meets solid
					columnFaceMasks[axis * 2 + 0][columnIndex] = column & ~(column >> 1);

					// Sample descending axis and set to true when air meets solid
					columnFaceMasks[axis * 2 + 1][columnIndex] = column & ~(column << 1);

					// Remove bottom face between height chunk if there are solid blocks underneath
					if (axis == 0) {
						if (bitIndex > 0) {
							const bool isFaceMaskSolid = columnFaceMasks[axis * 2 + 1][columnIndex] != 0;

							// Check if the leftmost bit is 1
							const uint16_t leftmostBitMask = 1U << 15;
							const bool isLeftmostBitSet = (binaryStats.yBinaryColumn[columnIndex - 1] & leftmostBitMask) != 0;

							// Remove bottom face if there are solid blocks beneath chunk
							if (isFaceMaskSolid && isLeftmostBitSet) {
								// Flip the rightmost bit to 0
								columnFaceMasks[axis * 2 + 1][columnIndex] &= ~1U;
							}
						} else {
							// Remove the bottom face of the world for the bottom chunk
							columnFaceMasks[axis * 2 + 1][columnIndex] &= ~1U;
						}
					}

				}
			}
		}
	}
}

void UStatsMeshGenerator::buildBinaryPlanes(const std::vector<uint16_t>& faceMaskColumn, std::vector<uint16_t>& binaryPlane, const int& axis) {
	for (int x = 0; x < StatsSizePadding; x++) {
		for (int z = 0; z < StatsSizePadding; z++) {
			for (int bitIndex = 0; bitIndex < StatsIntsPerHeight; bitIndex++) {

				const int columnIndex{ (x * StatsSizePadding * StatsIntsPerHeight) + (z * StatsIntsPerHeight) + bitIndex };

				uint16_t column = faceMaskColumn[columnIndex];

				// Remove padding only for X and Z axis 
				if (axis != 0 && axis != 1) {
					// Remove the leftmost bit and the rightmost bit and replace them with 0
					column = (column & ~(1U << 15)) & ~1U;
				}

				while (column != 0) {
					// Get the trailing zeros for the current column
					const int y = std::countr_zero(column);
					int planeIndex;
					int currentPlaneIndex;
					int planeRowIndex;
					switch (axis) {
					case 0:
					case 1:
						// Get to the correct plane and then add x to get to the correct row in the plane
						planeIndex = (y + bitIndex * StatsSizePadding) * StatsSizePadding + x;
						binaryPlane[planeIndex] |= (1U << z);
						break;
					case 2:
					case 3:
					case 4:
					case 5:
						planeRowIndex = y;
						if (columnIndex > 0) {
							currentPlaneIndex = (columnIndex / StatsSizePadding) * StatsSizePadding;
							planeIndex = currentPlaneIndex + planeRowIndex;
						} else {
							planeIndex = planeRowIndex;
						}

						binaryPlane[planeIndex] |= (1U << columnIndex % StatsSizePadding);
						break;
					}

					// Remove the padding from the plane
					binaryPlane[planeIndex] = (binaryPlane[planeIndex] & ~(1U << 15)) & ~1U;

					// Clear the position 
					column &= column - 1;
				}

			}
		}
	}
}

void UStatsMeshGenerator::greedyMeshingBinaryPlane(std::vector<uint16_t>& planes, const int& axis) {
	for (int row = 0; row < planes.size(); row++) {

		// Removing padding by skipping the first and last row in the plane
		if (row == 0) continue;
		else if (row % StatsSizePadding == 0 || row % StatsSizePadding == StatsSizePadding - 1) continue;

		while (planes[row] != 0) {
			// Get the starting point of the vertex
			const int y = std::countr_zero(planes[row]);

			// Trailing ones are the height of the vertex
			const int height = std::countr_one(static_cast<unsigned int>(planes[row]) >> y);

			uint16_t heightMask = ((1U << height) - 1) << y;

			// Flip the solid bits used to create the height mask 
			planes[row] = planes[row] & ~heightMask;

			int width = 1;
			int currentPlaneLimit{};

			// Get the expanding limit depending on the axis
			switch (axis) {
			case 0:
			case 1:
				currentPlaneLimit = StatsSize; // plane Y max limit is 64

				// Check if the next row can be expanded while in the bounds of the current plane 
				while ((row % StatsSizePadding) + width < currentPlaneLimit) {

					// Get the correct row to expand into, depending on the axis 
					const int planesIndex = row + width;

					// Get the bits spanning height for the next row
					const uint16_t nextRowHeight = planes[planesIndex] & heightMask;

					if (nextRowHeight != heightMask) {
						break; // Can't expand horizontally
					}

					// Remove the bits we expanded into
					planes[planesIndex] = planes[planesIndex] & ~heightMask;

					width++;
				}
				break;
			case 2:
			case 3:
			case 4:
			case 5:
				currentPlaneLimit = StatsHeight * StatsSize;

				// Check if the next row can be expanded while in the bounds of the current plane 
				while (row + (width * StatsSizePadding) < currentPlaneLimit) {

					// Get the row above the current one
					const int planesIndex = row + (width * StatsSizePadding);

					// Get the bits spanning height for the next row
					const uint16_t nextRowHeight = planes[planesIndex] & heightMask;

					if (nextRowHeight != heightMask) {
						break; // Can't expand horizontally
					}

					// Remove the bits we expanded into
					planes[planesIndex] = planes[planesIndex] & ~heightMask;

					width++;
				}
				break;
			}

			FVector voxelPosition1(3);
			FVector voxelPosition2(3);
			FVector voxelPosition3(3);
			FVector voxelPosition4(3);
			double voxelX{ 0.0 };
			double voxelZ{ 0.0 };

			switch (axis) {
			case 0:
			case 1:
			case 4:
			case 5:
				voxelX = static_cast<double>(row % StatsSizePadding);
				voxelZ = static_cast<double>(y);
				break;
			case 2:
			case 3:
				voxelZ = static_cast<double>(row % StatsSizePadding);
				voxelX = static_cast<double>(y);
				break;
			}

			const double voxelY = row > 0 ? std::floor(static_cast<double>(row / StatsSizePadding)) : 0.0;
			voxelPosition1 = { voxelX, voxelZ, voxelY }; // X, Y, Z (height)

			// Modify the original voxel position and create the remaining three quad position
			createAllVoxelPositionsFromOriginal(voxelPosition1, voxelPosition2, voxelPosition3, voxelPosition4, width, height, axis);

			// Create the quads
			createQuadAndAddToMeshData(voxelPosition1, voxelPosition2, voxelPosition3, voxelPosition4, width, height, axis);
		}
	}
}

void UStatsMeshGenerator::createAllVoxelPositionsFromOriginal(FVector& voxelPosition1, FVector& voxelPosition2, FVector& voxelPosition3, FVector& voxelPosition4, const int& width, const int& height, const int& axis) {
	// Get position modifiers depending on the current axis
	// This values are used to create the 4 quad positions
	FVector widthPositionModifier = { 0, 0, 0 };
	FVector heightPositionModifier = { 0, 0, 0 };

	// Vector for face directions
	FVector bottomFace = { 0, 0, 1 };
	FVector rightFace = { 1, 0, 0 };
	FVector backwardFace = { 0, 1, 0 };

	switch (axis) {
	case 0: // Y axis ascending
		widthPositionModifier[0] = width;
		heightPositionModifier[1] = height;

		// Adjust face direction
		voxelPosition1 += bottomFace;

		voxelPosition2 = voxelPosition1 + heightPositionModifier; // bottom - right
		voxelPosition3 = voxelPosition1 + widthPositionModifier + heightPositionModifier; // top - right
		voxelPosition4 = voxelPosition1 + widthPositionModifier;// top - left
		break;
	case 1: // Y axis descending
		widthPositionModifier[0] = width;
		heightPositionModifier[1] = height;

		voxelPosition2 = voxelPosition1 + widthPositionModifier; // top - left
		voxelPosition3 = voxelPosition1 + widthPositionModifier + heightPositionModifier; // top - right
		voxelPosition4 = voxelPosition1 + heightPositionModifier; // bottom - right
		break;
	case 2: // X axis ascending
		widthPositionModifier[0] = height;
		heightPositionModifier[2] = width;

		// Adjust face direction
		voxelPosition1 += backwardFace;

		voxelPosition2 = voxelPosition1 + widthPositionModifier; // top - left
		voxelPosition3 = voxelPosition1 + widthPositionModifier + heightPositionModifier; // top - right
		voxelPosition4 = voxelPosition1 + heightPositionModifier; // bottom - right
		break;
	case 3: // X axis descending
		widthPositionModifier[0] = height;
		heightPositionModifier[2] = width;

		voxelPosition2 = voxelPosition1 + heightPositionModifier; // bottom - right
		voxelPosition3 = voxelPosition1 + widthPositionModifier + heightPositionModifier; // top - right
		voxelPosition4 = voxelPosition1 + widthPositionModifier;// top - left
		break;
	case 4:	// Z axis ascending
		widthPositionModifier[1] = height;
		heightPositionModifier[2] = width;

		// Adjust face direction
		voxelPosition1 += rightFace;

		voxelPosition2 = voxelPosition1 + heightPositionModifier; // top - left
		voxelPosition3 = voxelPosition1 + widthPositionModifier + heightPositionModifier; // top - right
		voxelPosition4 = voxelPosition1 + widthPositionModifier; // bottom - right
		break;
	case 5: // Z axis descending
		widthPositionModifier[1] = height;
		heightPositionModifier[2] = width;

		voxelPosition2 = voxelPosition1 + widthPositionModifier; // bottom - right
		voxelPosition3 = voxelPosition1 + widthPositionModifier + heightPositionModifier; // top - right
		voxelPosition4 = voxelPosition1 + heightPositionModifier;// top - left
		break;
	default:
		UE_LOG(LogTemp, Error, TEXT("Invalid axis value: %d"), axis);
		ensureMsgf(false, TEXT("Unhandled case in switch statement for axis: %d"), axis);
		break;
	}
}

void UStatsMeshGenerator::createQuadAndAddToMeshData(FVector& voxelPosition1, FVector& voxelPosition2, FVector& voxelPosition3, FVector& voxelPosition4, const int& height, const int& width, const int& axis) {
	FVector Normal;
	if (axis == 0 || axis == 3 || axis == 5) {
		// Calculate the normals for counter clockwise vectors arrangement
		Normal = FVector::CrossProduct(voxelPosition4 - voxelPosition1, voxelPosition2 - voxelPosition1).GetSafeNormal();
	} else {
		// Calculate the normals for clockwise vectors arrangement
		Normal = FVector::CrossProduct(voxelPosition2 - voxelPosition1, voxelPosition4 - voxelPosition1).GetSafeNormal();
	}

	MeshData.Vertices.Append({
		voxelPosition1 * StatsScale,
		voxelPosition2 * StatsScale,
		voxelPosition3 * StatsScale,
		voxelPosition4 * StatsScale
		});

	// Add triangles and increment vertex count
	MeshData.Triangles.Append({
		vertexCount, vertexCount + 1, vertexCount + 2,
		vertexCount + 2, vertexCount + 3, vertexCount
		});

	vertexCount += 4;

	MeshData.Normals.Append({ Normal, Normal, Normal, Normal });

	// Invert the width with the height for the X and Z axis
	if (axis == 0 || axis == 1) {
		MeshData.UV0.Append({
			FVector2D(0, 0), FVector2D(0, width), FVector2D(height, width), FVector2D(height, 0)
			});
	} else {
		MeshData.UV0.Append({
			FVector2D(0, 0), FVector2D(0, height), FVector2D(width, height), FVector2D(width, 0)
			});
	}

	MeshData.Colors.Append({
		StatsColor,
		StatsColor,
		StatsColor,
		StatsColor
		});
}