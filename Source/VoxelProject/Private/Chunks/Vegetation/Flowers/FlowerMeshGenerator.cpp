#include "FlowerMeshGenerator.h"
#include <iostream>
#include <array>
#include <bitset>
#include <limits>
#include <string>
#include <set>
#include "..\..\TerrainSettings\WorldTerrainSettings.h"

UFlowerMeshGenerator::UFlowerMeshGenerator() {
}

UFlowerMeshGenerator::~UFlowerMeshGenerator() {
}


void UFlowerMeshGenerator::SetWorldTerrainSettings(UWorldTerrainSettings* InWorldTerrainSettings) {
	WorldTerrainSettingsRef = InWorldTerrainSettings;
}

void UFlowerMeshGenerator::createStemBinarySolidColumnsYXZ() {
	const FVector FlowerWorldLocation = FlowerLocationData.ObjectPosition;

	// Set the Flower values to air for all 3 axis (Y, X, Z)
	const int FlowerDimensions{ WTSR->FlowerSizePadding * WTSR->FlowerSizePadding * WTSR->FlowerIntsPerHeight };
	binaryFlower.yBinaryColumn = std::vector<uint16_t>(FlowerDimensions, 0);
	binaryFlower.xBinaryColumn = std::vector<uint16_t>(FlowerDimensions, 0);
	binaryFlower.zBinaryColumn = std::vector<uint16_t>(FlowerDimensions, 0);

	const int halfFlowerChunk = WTSR->FlowerSize / 2;

	// TODO Move this random generator to a different class, as this should be initialized only once
	// Create a random number generator
	std::random_device rd;
	std::mt19937 gen(rd());

	// Define the distribution
	std::uniform_int_distribution<> Flowerstems(3, 5);
	std::uniform_int_distribution<> FlowerHeight(3, WTSR->FlowerHeight - WTSR->MaxFlowerSphere);

	// This keeps it inside the 8x8 to fit a single voxel tile
	std::uniform_int_distribution<> FlowerPoint(4, WTSR->FlowerSize - 4);

	// Get the Flower stem voxels and add them to the binary columns
	const int TotalFlowerstems = Flowerstems(gen);

	TArray<FVector> stemPoints;

	int stemCounter = 0;

	// Add only unique random X and Z coordinates with random height
	std::set<std::pair<int, int>> uniquestemPoints;
	while (stemCounter < TotalFlowerstems) {
		const int x = FlowerPoint(gen);
		const int z = FlowerPoint(gen);

		// Check for direct neighbors to avoid stem touching
		if (uniquestemPoints.count({ x + 1, z }) || // Right
			uniquestemPoints.count({ x - 1, z }) || // Left
			uniquestemPoints.count({ x, z + 1 }) || // Above
			uniquestemPoints.count({ x, z - 1 })) { // Below
			continue;
		}

		// Add this point to the set
		uniquestemPoints.emplace(x, z);

		// Generate Flower stem points for this coordinate
		const int FlowerstemHeight = FlowerHeight(gen);

		for (int height = 0; height < FlowerstemHeight; height++) {
			FVector stemPoint(x, z, height);
			stemPoints.Add(stemPoint);

			// Add final stem point
			if (height == FlowerstemHeight - 1) {
				stemEndPoints.Add(stemPoint);
			}
		}

		stemCounter++;
	}


	for (int point = 0; point < stemPoints.Num(); point++) {
		int x = stemPoints[point].X;
		int z = stemPoints[point].Y;
		int height = stemPoints[point].Z;
		int localFlowerChunkHeight = (height % WTSR->FlowerHeight);
		int bitIndex = FMath::FloorToInt(height / (float)WTSR->FlowerSizePadding);

		// Get index of y 
		const int yIndex{ (x * WTSR->FlowerSizePadding * WTSR->FlowerIntsPerHeight) + (z * WTSR->FlowerIntsPerHeight) + bitIndex };

		// Add blocks height data (Y) to the current X and Z
		binaryFlower.yBinaryColumn[yIndex] |= (1U << localFlowerChunkHeight);

		const uint16_t currentYCol = binaryFlower.yBinaryColumn[yIndex];

		// Next Y index (column) means the same X index (column), but a change in Y bit index
		const int xIndex{ (height * WTSR->FlowerSizePadding) + x };

		binaryFlower.xBinaryColumn[xIndex] |= (1U << z);

		// Next Y index (column) means the next Z index (column), but the same Y bit index
		const int zIndex{ (height * WTSR->FlowerSizePadding) + z };

		// Assign to actual bit the change
		binaryFlower.zBinaryColumn[zIndex] |= (1U << x);
	}
}

void UFlowerMeshGenerator::createPetalsBinarySolidColumnsYXZ() {
	const FVector FlowerWorldLocation = FlowerLocationData.ObjectPosition;

	// Set the Flower values to air for all 3 axis (Y, X, Z)
	const int FlowerDimensions{ WTSR->FlowerSizePadding * WTSR->FlowerSizePadding * WTSR->FlowerIntsPerHeight };
	binaryFlower.yBinaryColumn = std::vector<uint16_t>(FlowerDimensions, 0);
	binaryFlower.xBinaryColumn = std::vector<uint16_t>(FlowerDimensions, 0);
	binaryFlower.zBinaryColumn = std::vector<uint16_t>(FlowerDimensions, 0);

	TArray<FVector> CrownPoints;

	// Generate crown at the end of Stem
	for (const FVector& endStemPoint : stemEndPoints) {
		GenerateSpherePoints(CrownPoints, endStemPoint.X, endStemPoint.Y, endStemPoint.Z);
	}

	for (int point = 0; point < CrownPoints.Num(); point++) {
		int x = CrownPoints[point].X;
		int z = CrownPoints[point].Y;
		int height = CrownPoints[point].Z;
		int localFlowerChunkHeight = (height % WTSR->FlowerHeight);
		int bitIndex = FMath::FloorToInt(height / (float)WTSR->FlowerSizePadding);

		// Get index of y 
		const int yIndex{ (x * WTSR->FlowerSizePadding * WTSR->FlowerIntsPerHeight) + (z * WTSR->FlowerIntsPerHeight) + bitIndex };

		// Add blocks height data (Y) to the current X and Z
		binaryFlower.yBinaryColumn[yIndex] |= (1U << localFlowerChunkHeight);

		const uint16_t currentYCol = binaryFlower.yBinaryColumn[yIndex];

		// Next Y index (column) means the same X index (column), but a change in Y bit index
		const int xIndex{ (height * WTSR->FlowerSizePadding) + x };

		binaryFlower.xBinaryColumn[xIndex] |= (1U << z);

		// Next Y index (column) means the next Z index (column), but the same Y bit index
		const int zIndex{ (height * WTSR->FlowerSizePadding) + z };

		// Assign to actual bit the change
		binaryFlower.zBinaryColumn[zIndex] |= (1U << x);
	}
}

//void UFlowerMeshGenerator::GenerateSpherePoints(TArray<FVector>& CrownPoints, const int& endX, const int& endZ, const int& endY) {
//	// Calculate the maximum possible radius based on the boundaries
//	int MaxXRadius = FMath::Min(2, WTSR->MaxFlowerSphere);  // Distance to nearest X boundary
//	int MaxZRadius = FMath::Min(2, WTSR->MaxFlowerSphere);  // Distance to nearest Z boundary
//	int MaxYRadius = FMath::Min(2, WTSR->MaxFlowerSphere); // Distance to nearest Y boundary
//
//	// The radius is the smallest distance to any of the boundaries
//	int SphereRadius = FMath::Min(MaxXRadius, FMath::Min(MaxZRadius, MaxYRadius));
//
//	// Number of steps for latitude and longitude
//	int LatitudeSteps = 300;
//	int LongitudeSteps = 300;
//
//	// Generate points along the sphere surface
//	for (int LatIndex = 0; LatIndex <= LatitudeSteps; LatIndex++) {
//		// Latitude angle (Phi) from 0 to PI
//		float Phi = PI * LatIndex / LatitudeSteps;
//
//		for (int LongIndex = 0; LongIndex <= LongitudeSteps; LongIndex++) {
//			// Longitude angle (Theta) from 0 to 2PI
//			float Theta = 2.0f * PI * LongIndex / LongitudeSteps;
//
//			// Convert spherical coordinates to Cartesian coordinates and apply a 3D noise 
//			float NoiseValue = FMath::PerlinNoise3D(FVector(Phi, Theta, SphereRadius)) * SphereRadius * 0.1f;
//			float X = (SphereRadius + NoiseValue) * FMath::Sin(Phi) * FMath::Cos(Theta);
//			float Z = (SphereRadius + NoiseValue) * FMath::Sin(Phi) * FMath::Sin(Theta);
//			float Y = (SphereRadius + NoiseValue) * FMath::Cos(Phi);
//
//			// Ensure all values are positive and fit within the Flower chunk
//			int FinalX = FMath::Clamp(FMath::RoundToInt(X + endX), 0, WTSR->FlowerSize);
//			int FinalZ = FMath::Clamp(FMath::RoundToInt(Z + endZ), 0, WTSR->FlowerSize);
//			int FinalY = FMath::Clamp(FMath::RoundToInt(Y + endY), 0, WTSR->FlowerHeight);
//
//			CrownPoints.Add(FVector(FinalX, FinalZ, FinalY));
//		}
//	}
//}

void UFlowerMeshGenerator::GenerateSpherePoints(TArray<FVector>& CrownPoints, const int& endX, const int& endZ, const int& endY) {
	// Define the sphere radius
	int SphereRadius = WTSR->MaxFlowerSphere;

	// Iterate through a bounding cube around the origin point
	for (int x = -SphereRadius; x <= SphereRadius; x++) {
		for (int z = -SphereRadius; z <= SphereRadius; z++) {
			for (int y = -SphereRadius; y <= SphereRadius; y++) {
				// Normalize the distance using an elliptical equation to make it more spherical
				float NormalizedDistance = (x * x) / (SphereRadius * SphereRadius) +
					(z * z) / (SphereRadius * SphereRadius) +
					(y * y) / (SphereRadius * SphereRadius);

				// Check if the point lies within the spherical boundary
				if (NormalizedDistance <= 1.0f) {
					// Convert local coordinates to global coordinates
					int FinalX = FMath::Clamp(endX + x, 0, WTSR->FlowerSize);
					int FinalZ = FMath::Clamp(endZ + z, 0, WTSR->FlowerSize);
					int FinalY = FMath::Clamp(endY + y, 0, WTSR->FlowerHeight);

					// Add the point to the array
					CrownPoints.Add(FVector(FinalX, FinalZ, FinalY));
				}
			}
		}
	}
}

void UFlowerMeshGenerator::AddStemPoints(TArray<FVector>& Points, float FlowerLength, float LastLayerProbability, int MaxBaseThickness) {
	int initialLength = Points.Num();

	for (int point = 0; point < initialLength; point++) {
		// The Stem should be thicker at the bottom
		int Thickness = FMath::Max(2, static_cast<int>(MaxBaseThickness * FMath::Sqrt(1 - (Points[point].Z - 1) / FlowerLength)));

		// Add points for the Stem
		for (int ThicknessPoint = 1; ThicknessPoint < Thickness; ThicknessPoint++) {
			// Add points to all the sides of the Stem
			for (int IncrementalThickness = -ThicknessPoint; IncrementalThickness <= ThicknessPoint; ++IncrementalThickness) {
				if (ThicknessPoint == Thickness - 1) {
					// Apply a random chance for the last layer
					if (FMath::FRand() < LastLayerProbability) {
						Points.Add(FVector(Points[point].X + ThicknessPoint, Points[point].Y + IncrementalThickness, Points[point].Z));
					}
					if (FMath::FRand() < LastLayerProbability) {
						Points.Add(FVector(Points[point].X - ThicknessPoint, Points[point].Y + IncrementalThickness, Points[point].Z));
					}
					if (FMath::FRand() < LastLayerProbability) {
						Points.Add(FVector(Points[point].X + IncrementalThickness, Points[point].Y + ThicknessPoint, Points[point].Z));
					}
					if (FMath::FRand() < LastLayerProbability) {
						Points.Add(FVector(Points[point].X + IncrementalThickness, Points[point].Y - ThicknessPoint, Points[point].Z));
					}
				} else {
					// Add all points for other layers
					Points.Add(FVector(Points[point].X + ThicknessPoint, Points[point].Y + IncrementalThickness, Points[point].Z));
					Points.Add(FVector(Points[point].X - ThicknessPoint, Points[point].Y + IncrementalThickness, Points[point].Z));
					Points.Add(FVector(Points[point].X + IncrementalThickness, Points[point].Y + ThicknessPoint, Points[point].Z));
					Points.Add(FVector(Points[point].X + IncrementalThickness, Points[point].Y - ThicknessPoint, Points[point].Z));
				}
			}
		}
	}
}


// Debugging function that prints a 64-bit integer in groups
void UFlowerMeshGenerator::printBinary(uint16_t value, int groupSize, const std::string& otherData) {
	// Ensure groupSize is a positive integer
	if (groupSize <= 0) {
		std::cerr << "Group size must be a positive integer" << std::endl;
		return;
	}

	// Convert the value to a 64-bit binary representation
	std::bitset<64> bits(value);
	std::string bitString = bits.to_string();

	// Initialize an empty string to hold the formatted output
	std::string formattedString;

	// Iterate over the bit string and add the delimiter after every groupSize bits
	for (size_t i = 0; i < bitString.size(); ++i) {
		formattedString += bitString[i];
		if ((i + 1) % groupSize == 0 && (i + 1) != bitString.size()) {
			formattedString += '|';
		}
	}

	// Convert the formatted string to an FString
	FString formattedFString = FString(formattedString.c_str());

	// Print the formatted binary string
	FString otherDataFString = FString(otherData.c_str());
	UE_LOG(LogTemp, Warning, TEXT("%s %s"), *otherDataFString, *formattedFString);

}

void UFlowerMeshGenerator::createTerrainMeshesData(bool forFlowerStem) {
	// Storing the face masks for the Y, X, Z axis
	// Size is doubled to contains both ascending and descending columns 
	std::vector<std::vector<uint16_t>> columnFaceMasks{
		std::vector<uint16_t>(WTSR->FlowerSizePadding * WTSR->FlowerSizePadding * WTSR->FlowerIntsPerHeight), // Y ascending
		std::vector<uint16_t>(WTSR->FlowerSizePadding * WTSR->FlowerSizePadding * WTSR->FlowerIntsPerHeight), // Y descending
		std::vector<uint16_t>(WTSR->FlowerSizePadding * WTSR->FlowerSizePadding * WTSR->FlowerIntsPerHeight), // X ascending
		std::vector<uint16_t>(WTSR->FlowerSizePadding * WTSR->FlowerSizePadding * WTSR->FlowerIntsPerHeight), // X descending
		std::vector<uint16_t>(WTSR->FlowerSizePadding * WTSR->FlowerSizePadding * WTSR->FlowerIntsPerHeight), // Z ascending
		std::vector<uint16_t>(WTSR->FlowerSizePadding * WTSR->FlowerSizePadding * WTSR->FlowerIntsPerHeight), // Z descending
	};

	// Face cull the binary columns on the 3 axis, ascending and descending
	faceCullingBinaryColumnsYXZ(columnFaceMasks);

	// Storing planes for all axis, ascending and descending
	std::vector<std::vector<uint16_t>> binaryPlanes{
		std::vector<uint16_t>(WTSR->FlowerSizePadding * WTSR->FlowerSizePadding * WTSR->FlowerIntsPerHeight), // Y ascending
		std::vector<uint16_t>(WTSR->FlowerSizePadding * WTSR->FlowerSizePadding * WTSR->FlowerIntsPerHeight), // Y descending
		std::vector<uint16_t>(WTSR->FlowerSizePadding * WTSR->FlowerSizePadding * WTSR->FlowerIntsPerHeight), // X ascending
		std::vector<uint16_t>(WTSR->FlowerSizePadding * WTSR->FlowerSizePadding * WTSR->FlowerIntsPerHeight), // X descending
		std::vector<uint16_t>(WTSR->FlowerSizePadding * WTSR->FlowerSizePadding * WTSR->FlowerIntsPerHeight), // Z ascending
		std::vector<uint16_t>(WTSR->FlowerSizePadding * WTSR->FlowerSizePadding * WTSR->FlowerIntsPerHeight), // Z descending
	};

	for (int axis = 0; axis < 6; axis++) { // Iterate all axis ascending and descending 
		// Create the binary plane for each axis
		buildBinaryPlanes(columnFaceMasks[axis], binaryPlanes[axis], axis);

		// Greedy mesh each plane and create planes
		greedyMeshingBinaryPlane(binaryPlanes[axis], axis, forFlowerStem);
	}
}

void UFlowerMeshGenerator::faceCullingBinaryColumnsYXZ(std::vector<std::vector<uint16_t>>& columnFaceMasks) {
	// Face culling for all the 3 axis (Y, X, Z)
	for (int axis = 0; axis < 3; axis++) {
		for (int x = 0; x < WTSR->FlowerSizePadding; x++) {
			for (int z = 0; z < WTSR->FlowerSizePadding; z++) {
				for (int bitIndex = 0; bitIndex < WTSR->FlowerIntsPerHeight; bitIndex++) {
					const int columnIndex{ (x * WTSR->FlowerSizePadding * WTSR->FlowerIntsPerHeight) + (z * WTSR->FlowerIntsPerHeight) + bitIndex };

					uint16_t column = 0;
					switch (axis) {
					case 0:
						column = binaryFlower.yBinaryColumn[columnIndex];
						break;
					case 1:
						column = binaryFlower.xBinaryColumn[columnIndex];
						break;
					case 2:
						column = binaryFlower.zBinaryColumn[columnIndex];
						break;
					}

					// If is the Y axis and not the last bitIndex
					if (axis == 0 && bitIndex < WTSR->FlowerIntsPerHeight - 1) {
						const bool isAboveSolid = binaryFlower.yBinaryColumn[columnIndex + 1] != 0;
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
							const bool isLeftmostBitSet = (binaryFlower.yBinaryColumn[columnIndex - 1] & leftmostBitMask) != 0;

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

void UFlowerMeshGenerator::buildBinaryPlanes(const std::vector<uint16_t>& faceMaskColumn, std::vector<uint16_t>& binaryPlane, const int& axis) {
	for (int x = 0; x < WTSR->FlowerSizePadding; x++) {
		for (int z = 0; z < WTSR->FlowerSizePadding; z++) {
			for (int bitIndex = 0; bitIndex < WTSR->FlowerIntsPerHeight; bitIndex++) {

				const int columnIndex{ (x * WTSR->FlowerSizePadding * WTSR->FlowerIntsPerHeight) + (z * WTSR->FlowerIntsPerHeight) + bitIndex }; // VERIFIED! Goes from 0 - 16,383

				uint16_t column = faceMaskColumn[columnIndex];  // this goes from 0 - 16,383

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
						planeIndex = (y + bitIndex * WTSR->FlowerSizePadding) * WTSR->FlowerSizePadding + x;
						binaryPlane[planeIndex] |= (1U << z);
						break;
					case 2:
					case 3:
					case 4:
					case 5:
						planeRowIndex = y;
						if (columnIndex > 0) {
							currentPlaneIndex = (columnIndex / WTSR->FlowerSizePadding) * WTSR->FlowerSizePadding;
							planeIndex = currentPlaneIndex + planeRowIndex;
						} else {
							planeIndex = planeRowIndex;
						}

						binaryPlane[planeIndex] |= (1U << columnIndex % WTSR->FlowerSizePadding);
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

void UFlowerMeshGenerator::greedyMeshingBinaryPlane(std::vector<uint16_t>& planes, const int& axis, bool forFlowerStem) {
	for (int row = 0; row < planes.size(); row++) {

		// Removing padding by skipping the first and last row in the plane
		if (row == 0) continue;
		else if (row % WTSR->FlowerSizePadding == 0 || row % WTSR->FlowerSizePadding == WTSR->FlowerSizePadding - 1) continue;

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
				currentPlaneLimit = WTSR->FlowerSize; // plane Y max limit is 64

				// Check if the next row can be expanded while in the bounds of the current plane 
				while ((row % WTSR->FlowerSizePadding) + width < currentPlaneLimit) {

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
				currentPlaneLimit = WTSR->FlowerHeight * WTSR->FlowerSize;

				// Check if the next row can be expanded while in the bounds of the current plane 
				while (row + (width * WTSR->FlowerSizePadding) < currentPlaneLimit) {

					// Get the row above the current one
					const int planesIndex = row + (width * WTSR->FlowerSizePadding);

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
				voxelX = static_cast<double>(row % WTSR->FlowerSizePadding);
				voxelZ = static_cast<double>(y);
				break;
			case 2:
			case 3:
				voxelZ = static_cast<double>(row % WTSR->FlowerSizePadding);
				voxelX = static_cast<double>(y);
				break;
			}

			// Height increases with each 64 rows for X and Z
			const double voxelY = row > 0 ? std::floor(static_cast<double>(row / WTSR->FlowerSizePadding)) : 0.0;
			voxelPosition1 = { voxelX, voxelZ, voxelY }; // X, Y, Z (height)

			// Modify the original voxel position and create the remaining three quad position
			createAllVoxelPositionsFromOriginal(voxelPosition1, voxelPosition2, voxelPosition3, voxelPosition4, width, height, axis, forFlowerStem);

			// Create the quads
			createQuadAndAddToMeshData(voxelPosition1, voxelPosition2, voxelPosition3, voxelPosition4, width, height, axis, forFlowerStem);
		}
	}
}

void UFlowerMeshGenerator::createAllVoxelPositionsFromOriginal(FVector& voxelPosition1, FVector& voxelPosition2, FVector& voxelPosition3, FVector& voxelPosition4, const int& width, const int& height, const int& axis, bool forFlowerStem) {
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

void UFlowerMeshGenerator::createQuadAndAddToMeshData(FVector& voxelPosition1, FVector& voxelPosition2, FVector& voxelPosition3, FVector& voxelPosition4, const int& height, const int& width, const int& axis, bool forFlowerStem) {
	FVector Normal;
	const FVector Edge1 = voxelPosition2 - voxelPosition1;
	const FVector Edge2 = voxelPosition4 - voxelPosition1;

	if (axis == 0 || axis == 2 || axis == 5 || axis == 4 || axis == 3) {
		Normal = FVector::CrossProduct(Edge2, Edge1);
	} else {
		Normal = FVector::CrossProduct(Edge1, Edge2);
	}

	Normal = Normal.GetSafeNormal();

	FColor layerColor;
	if (forFlowerStem) {
		layerColor = stemColor;
	} else {
		voxelPosition1 = voxelPosition1 + 0.001f;
		voxelPosition2 = voxelPosition2 + 0.001f;
		voxelPosition3 = voxelPosition3 + 0.001f;
		voxelPosition4 = voxelPosition4 + 0.001f;
		layerColor = petalsColor;
	}

	MeshData.Vertices.Append({
		voxelPosition1 * WTSR->FlowerScale,
		voxelPosition2 * WTSR->FlowerScale,
		voxelPosition3 * WTSR->FlowerScale,
		voxelPosition4 * WTSR->FlowerScale
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

FVoxelObjectMeshData UFlowerMeshGenerator::GetFlowerMeshData(const FColor& InStemColor, const FColor& InPetalsColor) {

	// Select Stem and crown colors
	stemColor = InStemColor;
	petalsColor = InPetalsColor;

	createStemBinarySolidColumnsYXZ();
	createTerrainMeshesData(true);

	createPetalsBinarySolidColumnsYXZ();
	createTerrainMeshesData(false);

	FVoxelObjectMeshData TemporaryMeshData = MeshData;

	// Resetting the data structures
	MeshData.Vertices.Empty();
	MeshData.Triangles.Empty();
	MeshData.Normals.Empty();
	MeshData.Colors.Empty();
	MeshData.UV0.Empty();
	binaryFlower = BinaryFlower3D{};
	binaryFlower.yBinaryColumn.clear();
	binaryFlower.xBinaryColumn.clear();
	binaryFlower.zBinaryColumn.clear();
	columnsFaceMask.clear();
	stemEndPoints.Empty();
	FlowerLocationData.ObjectPosition = FVector::ZeroVector; 
	FlowerLocationData.ObjectWorldCoords = FIntPoint(0, 0); 
	vertexCount = 0;

	return TemporaryMeshData;
}