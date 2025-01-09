#include "GrassMeshGenerator.h"
#include <iostream>
#include <array>
#include <bitset>
#include <limits>
#include <set>
#include <string>
#include "..\..\TerrainSettings\WorldTerrainSettings.h"

UGrassMeshGenerator::UGrassMeshGenerator() {
}

UGrassMeshGenerator::~UGrassMeshGenerator() {
}


void UGrassMeshGenerator::SetWorldTerrainSettings(UWorldTerrainSettings* InWorldTerrainSettings) {
	WorldTerrainSettingsRef = InWorldTerrainSettings;
}

void UGrassMeshGenerator::createBladeBinarySolidColumnsYXZ() {
	const FVector GrassWorldLocation = GrassLocationData.ObjectPosition;

	// Set the Grass values to air for all 3 axis (Y, X, Z)
	const int GrassDimensions{ WTSR->GrassSizePadding * WTSR->GrassSizePadding * WTSR->GrassIntsPerHeight };
	binaryGrass.yBinaryColumn = std::vector<uint8_t>(GrassDimensions, 0);
	binaryGrass.xBinaryColumn = std::vector<uint8_t>(GrassDimensions, 0);
	binaryGrass.zBinaryColumn = std::vector<uint8_t>(GrassDimensions, 0);

	const int halfGrassChunk = WTSR->GrassSize / 2;

	// TODO Move this random generator to a different class, as this should be initialized only once
	// Create a random number generator
	std::random_device rd;
	std::mt19937 gen(rd());

	// Define the distribution
	std::uniform_int_distribution<> GrassBlades(5, 15);
	std::uniform_int_distribution<> GrassHeight(3, WTSR->GrassHeight);
	std::uniform_int_distribution<> GrassPoint(1, WTSR->GrassSize);

	// Get the Grass blade voxels and add them to the binary columns
	const int TotalGrassBlades = GrassBlades(gen);

	TArray<FVector> BladePoints;

	int bladeCounter = 0;

	// Add only unique random X and Z coordinates with random height
	std::set<std::pair<int, int>> uniqueBladePoints; 
	while (bladeCounter < TotalGrassBlades) {

		const int x = GrassPoint(gen);
		const int z = GrassPoint(gen);

		// Continue only if x and z weren't used before
		if (uniqueBladePoints.emplace(x, z).second) {
			const int GrassBladeHeight = GrassHeight(gen);

			// Create all the points for the Grass Blade
			for (int height = 0; height < GrassBladeHeight; height++) {
				FVector BladePoint(x, z, height);

				BladePoints.Add(BladePoint);
			}

			bladeCounter++;
		}
	}

	for (int point = 0; point < BladePoints.Num(); point++) {
		int x = BladePoints[point].X;
		int z = BladePoints[point].Y;
		int height = BladePoints[point].Z;
		int localGrassChunkHeight = (height % WTSR->GrassHeight);
		int bitIndex = FMath::FloorToInt(height / (float)WTSR->GrassSizePadding);

		// Get index of y 
		const int yIndex{ (x * WTSR->GrassSizePadding * WTSR->GrassIntsPerHeight) + (z * WTSR->GrassIntsPerHeight) + bitIndex };

		// Add blocks height data (Y) to the current X and Z
		binaryGrass.yBinaryColumn[yIndex] |= (1U << localGrassChunkHeight);

		const uint8_t currentYCol = binaryGrass.yBinaryColumn[yIndex];

		// Next Y index (column) means the same X index (column), but a change in Y bit index
		const int xIndex{ (height * WTSR->GrassSizePadding) + x }; 

		binaryGrass.xBinaryColumn[xIndex] |= (1U << z);

		// Next Y index (column) means the next Z index (column), but the same Y bit index
		const int zIndex{ (height * WTSR->GrassSizePadding) + z }; 

		// Assign to actual bit the change
		binaryGrass.zBinaryColumn[zIndex] |= (1U << x);
	}
}

void UGrassMeshGenerator::createCrownBinarySolidColumnsYXZ() {
	const FVector GrassWorldLocation = GrassLocationData.ObjectPosition;

	// Set the Grass values to air for all 3 axis (Y, X, Z)
	const int GrassDimensions{ WTSR->GrassSizePadding * WTSR->GrassSizePadding * WTSR->GrassIntsPerHeight };
	binaryGrass.yBinaryColumn = std::vector<uint8_t>(GrassDimensions, 0);
	binaryGrass.xBinaryColumn = std::vector<uint8_t>(GrassDimensions, 0);
	binaryGrass.zBinaryColumn = std::vector<uint8_t>(GrassDimensions, 0);

	TArray<FVector> CrownPoints;

	for (int point = 0; point < CrownPoints.Num(); point++) {
		int x = CrownPoints[point].X;
		int z = CrownPoints[point].Y;
		int height = CrownPoints[point].Z;
		int localGrassChunkHeight = (height % WTSR->GrassHeight);
		int bitIndex = FMath::FloorToInt(height / (float)WTSR->GrassSizePadding);

		// Get index of y 
		const int yIndex{ (x * WTSR->GrassSizePadding * WTSR->GrassIntsPerHeight) + (z * WTSR->GrassIntsPerHeight) + bitIndex };

		// Add blocks height data (Y) to the current X and Z
		binaryGrass.yBinaryColumn[yIndex] |= (1U << localGrassChunkHeight);

		const uint8_t currentYCol = binaryGrass.yBinaryColumn[yIndex];

		// Next Y index (column) means the same X index (column), but a change in Y bit index
		const int xIndex{ (height * WTSR->GrassSizePadding) + x };

		binaryGrass.xBinaryColumn[xIndex] |= (1U << z);

		// Next Y index (column) means the next Z index (column), but the same Y bit index
		const int zIndex{ (height * WTSR->GrassSizePadding) + z };

		// Assign to actual bit the change
		binaryGrass.zBinaryColumn[zIndex] |= (1U << x);
	}
}

void UGrassMeshGenerator::createTerrainMeshesData() {
	// Storing the face masks for the Y, X, Z axis
	// Size is doubled to contains both ascending and descending columns 
	std::vector<std::vector<uint8_t>> columnFaceMasks{
		std::vector<uint8_t>(WTSR->GrassSizePadding * WTSR->GrassSizePadding * WTSR->GrassIntsPerHeight), // Y ascending
		std::vector<uint8_t>(WTSR->GrassSizePadding * WTSR->GrassSizePadding * WTSR->GrassIntsPerHeight), // Y descending
		std::vector<uint8_t>(WTSR->GrassSizePadding * WTSR->GrassSizePadding * WTSR->GrassIntsPerHeight), // X ascending
		std::vector<uint8_t>(WTSR->GrassSizePadding * WTSR->GrassSizePadding * WTSR->GrassIntsPerHeight), // X descending
		std::vector<uint8_t>(WTSR->GrassSizePadding * WTSR->GrassSizePadding * WTSR->GrassIntsPerHeight), // Z ascending
		std::vector<uint8_t>(WTSR->GrassSizePadding * WTSR->GrassSizePadding * WTSR->GrassIntsPerHeight), // Z descending
	};

	// Face cull the binary columns on the 3 axis, ascending and descending
	faceCullingBinaryColumnsYXZ(columnFaceMasks);

	// Storing planes for all axis, ascending and descending
	std::vector<std::vector<uint8_t>> binaryPlanes{
		std::vector<uint8_t>(WTSR->GrassSizePadding * WTSR->GrassSizePadding * WTSR->GrassIntsPerHeight), // Y ascending
		std::vector<uint8_t>(WTSR->GrassSizePadding * WTSR->GrassSizePadding * WTSR->GrassIntsPerHeight), // Y descending
		std::vector<uint8_t>(WTSR->GrassSizePadding * WTSR->GrassSizePadding * WTSR->GrassIntsPerHeight), // X ascending
		std::vector<uint8_t>(WTSR->GrassSizePadding * WTSR->GrassSizePadding * WTSR->GrassIntsPerHeight), // X descending
		std::vector<uint8_t>(WTSR->GrassSizePadding * WTSR->GrassSizePadding * WTSR->GrassIntsPerHeight), // Z ascending
		std::vector<uint8_t>(WTSR->GrassSizePadding * WTSR->GrassSizePadding * WTSR->GrassIntsPerHeight), // Z descending
	};

	for (int axis = 0; axis < 6; axis++) { // Iterate all axis ascending and descending 
		// Create the binary plane for each axis
		buildBinaryPlanes(columnFaceMasks[axis], binaryPlanes[axis], axis);

		// Greedy mesh each plane and create planes
		greedyMeshingBinaryPlane(binaryPlanes[axis], axis);
	}
}

void UGrassMeshGenerator::faceCullingBinaryColumnsYXZ(std::vector<std::vector<uint8_t>>& columnFaceMasks) {
	// Face culling for all the 3 axis (Y, X, Z)
	for (int axis = 0; axis < 3; axis++) {
		for (int x = 0; x < WTSR->GrassSizePadding; x++) {
			for (int z = 0; z < WTSR->GrassSizePadding; z++) {
				for (int bitIndex = 0; bitIndex < WTSR->GrassIntsPerHeight; bitIndex++) {
					const int columnIndex{ (x * WTSR->GrassSizePadding * WTSR->GrassIntsPerHeight) + (z * WTSR->GrassIntsPerHeight) + bitIndex };

					uint8_t column = 0;
					switch (axis) {
					case 0:
						column = binaryGrass.yBinaryColumn[columnIndex];
						break;
					case 1:
						column = binaryGrass.xBinaryColumn[columnIndex];
						break;
					case 2:
						column = binaryGrass.zBinaryColumn[columnIndex];
						break;
					}

					// If is the Y axis and not the last bitIndex
					if (axis == 0 && bitIndex < WTSR->GrassIntsPerHeight - 1) {
						const bool isAboveSolid = binaryGrass.yBinaryColumn[columnIndex + 1] != 0;
						const bool columnAllSolid = column == std::numeric_limits<uint8_t>::max();

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
							const uint8_t leftmostBitMask = 1U << 7;
							const bool isLeftmostBitSet = (binaryGrass.yBinaryColumn[columnIndex - 1] & leftmostBitMask) != 0;

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

void UGrassMeshGenerator::buildBinaryPlanes(const std::vector<uint8_t>& faceMaskColumn, std::vector<uint8_t>& binaryPlane, const int& axis) {
	for (int x = 0; x < WTSR->GrassSizePadding; x++) {
		for (int z = 0; z < WTSR->GrassSizePadding; z++) {
			for (int bitIndex = 0; bitIndex < WTSR->GrassIntsPerHeight; bitIndex++) {

				const int columnIndex{ (x * WTSR->GrassSizePadding * WTSR->GrassIntsPerHeight) + (z * WTSR->GrassIntsPerHeight) + bitIndex }; // VERIFIED! Goes from 0 - 16,383

				uint8_t column = faceMaskColumn[columnIndex];  // this goes from 0 - 16,383

				// Remove padding only for X and Z axis 
				if (axis != 0 && axis != 1) {
					// Remove the leftmost bit and the rightmost bit and replace them with 0
					column = (column & ~(1U << 7)) & ~1U;
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
						planeIndex = (y + bitIndex * WTSR->GrassSizePadding) * WTSR->GrassSizePadding + x;
						binaryPlane[planeIndex] |= (1U << z);
						break;
					case 2:
					case 3:
					case 4:
					case 5:
						planeRowIndex = y;
						if (columnIndex > 0) {
							currentPlaneIndex = (columnIndex / WTSR->GrassSizePadding) * WTSR->GrassSizePadding;
							planeIndex = currentPlaneIndex + planeRowIndex;
						} else {
							planeIndex = planeRowIndex;
						}

						binaryPlane[planeIndex] |= (1U << columnIndex % WTSR->GrassSizePadding);
						break;
					}

					// Remove the padding from the plane
					binaryPlane[planeIndex] = (binaryPlane[planeIndex] & ~(1U << 7)) & ~1U;

					// Clear the position 
					column &= column - 1;
				}

			}
		}
	}
}

void UGrassMeshGenerator::greedyMeshingBinaryPlane(std::vector<uint8_t>& planes, const int& axis) {
	for (int row = 0; row < planes.size(); row++) {

		// Removing padding by skipping the first and last row in the plane
		if (row == 0) continue;
		else if (row % WTSR->GrassSizePadding == 0 || row % WTSR->GrassSizePadding == WTSR->GrassSizePadding - 1) continue;

		while (planes[row] != 0) {
			// Get the starting point of the vertex
			const int y = std::countr_zero(planes[row]);

			// Trailing ones are the height of the vertex
			const int height = std::countr_one(static_cast<unsigned int>(planes[row]) >> y);

			uint8_t heightMask = ((1U << height) - 1) << y;

			// Flip the solid bits used to create the height mask 
			planes[row] = planes[row] & ~heightMask;

			int width = 1;
			int currentPlaneLimit{};

			// Get the expanding limit depending on the axis
			switch (axis) {
			case 0:
			case 1:
				currentPlaneLimit = WTSR->GrassSize; // plane Y max limit is 64

				// Check if the next row can be expanded while in the bounds of the current plane 
				while ((row % WTSR->GrassSizePadding) + width < currentPlaneLimit) {

					// Get the correct row to expand into, depending on the axis 
					const int planesIndex = row + width;

					// Get the bits spanning height for the next row
					const uint8_t nextRowHeight = planes[planesIndex] & heightMask;

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
				currentPlaneLimit = WTSR->GrassHeight * WTSR->GrassSize;

				// Check if the next row can be expanded while in the bounds of the current plane 
				while (row + (width * WTSR->GrassSizePadding) < currentPlaneLimit) {

					// Get the row above the current one
					const int planesIndex = row + (width * WTSR->GrassSizePadding);

					// Get the bits spanning height for the next row
					const uint8_t nextRowHeight = planes[planesIndex] & heightMask;

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
				voxelX = static_cast<double>(row % WTSR->GrassSizePadding);
				voxelZ = static_cast<double>(y);
				break;
			case 2:
			case 3:
				voxelZ = static_cast<double>(row % WTSR->GrassSizePadding);
				voxelX = static_cast<double>(y);
				break;
			}

			// Height increases with each 64 rows for X and Z
			const double voxelY = row > 0 ? std::floor(static_cast<double>(row / WTSR->GrassSizePadding)) : 0.0;
			voxelPosition1 = { voxelX, voxelZ, voxelY }; // X, Y, Z (height)

			// Modify the original voxel position and create the remaining three quad position
			createAllVoxelPositionsFromOriginal(voxelPosition1, voxelPosition2, voxelPosition3, voxelPosition4, width, height, axis);

			// Create the quads
			createQuadAndAddToMeshData(voxelPosition1, voxelPosition2, voxelPosition3, voxelPosition4, width, height, axis);
		}
	}
}

void UGrassMeshGenerator::createAllVoxelPositionsFromOriginal(FVector& voxelPosition1, FVector& voxelPosition2, FVector& voxelPosition3, FVector& voxelPosition4, const int& width, const int& height, const int& axis) {
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

void UGrassMeshGenerator::createQuadAndAddToMeshData(FVector& voxelPosition1, FVector& voxelPosition2, FVector& voxelPosition3, FVector& voxelPosition4, const int& height, const int& width, const int& axis) {
	FVector Normal;
	if (axis == 0 || axis == 3 || axis == 5) {
		// Calculate the normals for counter clockwise vectors arrangement
		Normal = FVector::CrossProduct(voxelPosition4 - voxelPosition1, voxelPosition2 - voxelPosition1).GetSafeNormal();
	} else {
		// Calculate the normals for clockwise vectors arrangement
		Normal = FVector::CrossProduct(voxelPosition2 - voxelPosition1, voxelPosition4 - voxelPosition1).GetSafeNormal();
	}

	FColor layerColor = WTSR->GrassBladesColorArray[FMath::RandRange(0, WTSR->GrassBladesColorArray.Num() - 1)];;

	MeshData.Vertices.Append({
		voxelPosition1 * WTSR->GrassScale,
		voxelPosition2 * WTSR->GrassScale,
		voxelPosition3 * WTSR->GrassScale,
		voxelPosition4 * WTSR->GrassScale
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
		layerColor,
		layerColor,
		layerColor,
		layerColor
		});
}

FVoxelObjectMeshData UGrassMeshGenerator::GetGrassMeshData() {

	createBladeBinarySolidColumnsYXZ();
	createTerrainMeshesData();

	FVoxelObjectMeshData TemporaryMeshData = MeshData;

	// Resetting the data structures
	MeshData.Vertices.Empty();
	MeshData.Triangles.Empty();
	MeshData.Normals.Empty();
	MeshData.Colors.Empty();
	MeshData.UV0.Empty();
	binaryGrass = BinaryGrass3D{};
	binaryGrass.yBinaryColumn.clear();
	binaryGrass.xBinaryColumn.clear();
	binaryGrass.zBinaryColumn.clear();
	columnsFaceMask.clear();
	branchEndPoints.Empty();
	GrassLocationData.ObjectPosition = FVector::ZeroVector; 
	GrassLocationData.ObjectWorldCoords = FIntPoint(0, 0); 
	vertexCount = 0;

	return TemporaryMeshData;
}