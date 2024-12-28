

#include "Tree.h"
#include <iostream>
#include <array>
#include <bitset>
#include <limits>
#include <string>

#include "..\..\TerrainSettings\WorldTerrainSettings.h"
#include "ProceduralMeshComponent.h"

// Sets default values
ATree::ATree() {
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	Mesh = CreateDefaultSubobject<UProceduralMeshComponent>("Mesh");

	Mesh->SetCastShadow(false);

	// By default the tree shouldn't have collision
	hasCollision = false;

	// Set mesh as root
	SetRootComponent(Mesh);
}

ATree::~ATree() {
}

void ATree::SetTreeCollision(bool InHasCollision) {
	hasCollision = InHasCollision;
}

void ATree::SetWorldTerrainSettings(UWorldTerrainSettings* InWorldTerrainSettings) {
	WorldTerrainSettingsRef = InWorldTerrainSettings;
}

void ATree::SetPerlinNoiseSettings(APerlinNoiseSettings* InPerlinNoiseSettings) {
	PerlinNoiseSettingsRef = InPerlinNoiseSettings;
}

void ATree::printExecutionTime(Time& start, Time& end, const char* functionName) {
	std::chrono::duration<double, std::milli> duration = end - start;
	UE_LOG(LogTemp, Warning, TEXT("%s() took %f milliseconds to execute."), *FString(functionName), duration.count());
}

void ATree::createTrunkBinarySolidColumnsYXZ() {
	const FVector treeWorldLocation = TreeLocationData.ObjectPosition;

	// Set the tree values to air for all 3 axis (Y, X, Z)
	const int treeDimensions{ WTSR->TreeSizePadding * WTSR->TreeSizePadding * WTSR->TreeIntsPerHeight };
	binaryTree.yBinaryColumn = std::vector<uint64_t>(treeDimensions, 0);
	binaryTree.xBinaryColumn = std::vector<uint64_t>(treeDimensions, 0);
	binaryTree.zBinaryColumn = std::vector<uint64_t>(treeDimensions, 0);

	const int halfTreeChunk = WTSR->TreeSize / 2;

	// TODO Move this random generator to a different class, as this should be initialized only once
	// Create a random number generator
	std::random_device rd;  
	std::mt19937 gen(rd());

	// Define the distribution
	std::uniform_int_distribution<> treeTrunkHeightDistribution(WTSR->TreeHeight / 2, WTSR->TreeHeight);
	std::uniform_int_distribution<> treeBranchDistribution(1, 4);
	std::uniform_int_distribution<> treeBranchLengthDistribution(10, 20);
	std::uniform_int_distribution<> treeTrunkSizeDistribution(WTSR->MinTreeTrunkWidth, WTSR->MaxTreeTrunkWidth);
	std::uniform_real_distribution<> branchStartPoint(0.5, 0.8);

	std::uniform_int_distribution<> branchPointLocation(WTSR->MaxTreeTrunkWidth, WTSR->TreeSize - WTSR->MaxTreeTrunkWidth);

	// Get the tree trunk voxels and add them to the binary columns
	const int treeTrunkHeight = treeTrunkHeightDistribution(gen);
	const int treeBranches = treeBranchDistribution(gen);
	const int treeTrunkWidth = treeTrunkSizeDistribution(gen);

	TArray<FVector> TrunkPoints;
	
	// Create all the points for the tree trunk
	for (int height = 0; height < treeTrunkHeight; height++) {

		FVector trunkPoint(halfTreeChunk, halfTreeChunk, height);

		TrunkPoints.Add(trunkPoint);
	}

	
	TArray<FVector> branchEndPoints;
	// Add the branches points
	for (int branch = 0; branch < treeBranches; branch++) {

		const int randomX = branchPointLocation(gen);
		const int randomZ = branchPointLocation(gen);

		// Add the points that connect the trunk with the final branch point
		int startX = halfTreeChunk;
		int startZ = halfTreeChunk;

		float startBranchPoint = branchStartPoint(gen);
		int startY = FMath::FloorToInt(treeTrunkHeight * startBranchPoint);


		std::uniform_int_distribution<> branchHeight(startY, treeTrunkHeight);
		int endBranchPoint = branchHeight(gen);

		branchEndPoints.Add(FVector(randomX, randomZ, endBranchPoint));

		while (startX != randomX || startZ != randomZ || startY != endBranchPoint) {

			if (startX != randomX) {
				startX += (startX > randomX) ? -1 : 1;
			}

			if (startZ != randomZ) {
				startZ += (startZ > randomZ) ? -1 : 1;
			}

			if (startY != endBranchPoint) {
				startY += (startY > endBranchPoint) ? -1 : 1;
			}

			FVector branchPoint(startX, startZ, startY);
			TrunkPoints.Add(branchPoint);
		}

	}

	// Increasing the trunk width by adding more points
	AddTrunkPoints(TrunkPoints, treeTrunkHeight, static_cast<float>(.9), treeTrunkWidth);

	// Generate crown at the end of branch
	for (const FVector& endBranchPoint : branchEndPoints) {
		GenerateSpherePoints(TrunkPoints, endBranchPoint.X, endBranchPoint.Y, endBranchPoint.Z);
	}

	for (int point = 0; point < TrunkPoints.Num(); point++) {
		int x = TrunkPoints[point].X;
		int z = TrunkPoints[point].Y;
		int height = TrunkPoints[point].Z;
		int localTreeChunkHeight = (height % WTSR->TreeHeight);
		int bitIndex = FMath::FloorToInt(height / (float)WTSR->TreeSizePadding);

		// Get index of y 
		const int yIndex{ (x * WTSR->TreeSizePadding * WTSR->TreeIntsPerHeight) + (z * WTSR->TreeIntsPerHeight) + bitIndex };

		//UE_LOG(LogTemp, Warning, TEXT("Current point : X=%f, Z=%f, Y=%f"), TrunkPoints[point].X, TrunkPoints[point].Y, TrunkPoints[point].Z);

		//printBinary(binaryTree.yBinaryColumn[yIndex], 8, "yIndex " + std::to_string(yIndex) + " Y column BEFORE : ");
		 
		// Add blocks height data (Y) to the current X and Z
		binaryTree.yBinaryColumn[yIndex] |= (1ULL << localTreeChunkHeight);

		//printBinary(binaryTree.yBinaryColumn[yIndex], 8, "yIndex " + std::to_string(yIndex) + " Y column AFTER : ");

		const uint64_t currentYCol = binaryTree.yBinaryColumn[yIndex];

		// Next Y index (column) means the same X index (column), but a change in Y bit index
		const int xIndex{ (height * WTSR->TreeSizePadding) + x }; // (bitIndex * WTSR->TreeSizePadding * WTSR->TreeSizePadding)

		//printBinary(binaryTree.xBinaryColumn[xIndex], 8, "xIndex " + std::to_string(xIndex) + " X column BEFORE : ");

		binaryTree.xBinaryColumn[xIndex] |= (1ULL << z);

		//printBinary(binaryTree.xBinaryColumn[xIndex], 8, "xIndex " + std::to_string(xIndex) + " X column AFTER : ");

		// Next Y index (column) means the next Z index (column), but the same Y bit index
		const int zIndex{ (height * WTSR->TreeSizePadding) + z }; // + (bitIndex * WTSR->TreeSizePadding * WTSR->TreeSizePadding)

		//printBinary(binaryTree.zBinaryColumn[zIndex], 8, "zIndex " + std::to_string(zIndex) + " Z column BEFORE : ");

		// Assign to actual bit the change
		binaryTree.zBinaryColumn[zIndex] |= (1ULL << x);

		//printBinary(binaryTree.zBinaryColumn[zIndex], 8, "zIndex " + std::to_string(zIndex) + " Z column AFTER : ");
	}
}

void ATree::GenerateSpherePoints(TArray<FVector>& CrownPoints, const int& endX, const int& endZ, const int& endY) {
	// Randomly determine the radius of the sphere
	int SphereRadius = FMath::RandRange(15, 20); // Radius is limited to fit the box

	// Specify the resolution for latitude and longitude
	int LatitudeSteps = 300;  // Number of steps for latitude
	int LongitudeSteps = 300; // Number of steps for longitude

	// CrownPoints.Empty(); // Ensure the array is empty before adding points

	// Generate points along the sphere surface
	for (int LatIndex = 0; LatIndex <= LatitudeSteps; ++LatIndex) {
		// Latitude angle (Phi) from 0 to PI
		float Phi = PI * LatIndex / LatitudeSteps;

		for (int LongIndex = 0; LongIndex <= LongitudeSteps; ++LongIndex) {
			// Longitude angle (Theta) from 0 to 2PI
			float Theta = 2.0f * PI * LongIndex / LongitudeSteps;

			// Convert spherical coordinates to Cartesian coordinates
			float X = SphereRadius * FMath::Sin(Phi) * FMath::Cos(Theta);
			float Z = SphereRadius * FMath::Sin(Phi) * FMath::Sin(Theta);
			float Y = SphereRadius * FMath::Cos(Phi);

			// Ensure all values are positive and fit within the box
			int FinalX = FMath::Clamp(FMath::RoundToInt(X + SphereRadius) + endX, 0, WTSR->TreeSize);  // Offset by radius for positive values
			int FinalZ = FMath::Clamp(FMath::RoundToInt(Z + SphereRadius) + endZ, 0, WTSR->TreeSize);
			int FinalY = FMath::Clamp(FMath::RoundToInt(Y + SphereRadius) + endY, 0, WTSR->TreeHeight);

			// Add the point to the array
			CrownPoints.Add(FVector(FinalX, FinalZ, FinalY));
		}
	}
}

void ATree::AddTrunkPoints(TArray<FVector>& Points, float TreeLength, float LastLayerProbability, int MaxBaseThickness) {
	int initialLength = Points.Num();

	for (int point = 0; point < initialLength; point++) {
		// The trunk should be thicker at the bottom
		int Thickness = FMath::Max(2, static_cast<int>(MaxBaseThickness * FMath::Sqrt(1 - (Points[point].Z - 1) / TreeLength)));

		// Add points for the trunk
		for (int ThicknessPoint = 1; ThicknessPoint < Thickness; ThicknessPoint++) {
			// Add points to all the sides of the trunk
			for (int IncrementalThickness = -ThicknessPoint; IncrementalThickness <= ThicknessPoint; ++IncrementalThickness) {
				if (ThicknessPoint == Thickness - 1) {
					// Apply random chance for the last layer
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
void ATree::printBinary(uint64_t value, int groupSize, const std::string& otherData) {
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

void ATree::apply3DNoiseToHeightColumn(uint64_t& column, int& x, int& z, int& y, int& bitIndex, const FVector& treeWorldLocation, int& height) {
	// Return early if total height is bigger than the threshold
	if (height > 50) {
		return;
	}

	// Getting perlin noise position, adjusted to the Unreal Engine grid system 
	const float noisePositionX = static_cast<float>((x * WTSR->TreeScale + treeWorldLocation.X) / WTSR->TreeScale);
	const float noisePositionZ = static_cast<float>((z * WTSR->TreeScale + treeWorldLocation.Y) / WTSR->TreeScale);
	const float noisePositionY = static_cast<float>(((y + bitIndex * WTSR->TreeSizePadding) * WTSR->TreeScale + treeWorldLocation.Z) / WTSR->TreeScale);

	const float squashingFactor = (y + bitIndex * WTSR->TreeSizePadding) * PNSR->squashingFactor;

	// noise->SetFrequency(PNSR->noiseFrequency3D);

	// const float noiseValue3D = noise->GetNoise(noisePositionX, noisePositionZ, noisePositionY) + 1;

	// TODO Add 3d noise depending on the height of the actual 2D noise
	if (y + bitIndex * WTSR->TreeSizePadding < 50 && height < 50) { // y + bitIndex * WTSR->TreeSizePadding > 100)
		column |= 1ULL << y;
	}
}

void ATree::createTerrainMeshesData() {
	// Storing the face masks for the Y, X, Z axis
	// Size is doubled to contains both ascending and descending columns 
	std::vector<std::vector<uint64_t>> columnFaceMasks{
		std::vector<uint64_t>(WTSR->TreeSizePadding * WTSR->TreeSizePadding * WTSR->TreeIntsPerHeight), // Y ascending
		std::vector<uint64_t>(WTSR->TreeSizePadding * WTSR->TreeSizePadding * WTSR->TreeIntsPerHeight), // Y descending
		std::vector<uint64_t>(WTSR->TreeSizePadding * WTSR->TreeSizePadding * WTSR->TreeIntsPerHeight), // X ascending
		std::vector<uint64_t>(WTSR->TreeSizePadding * WTSR->TreeSizePadding * WTSR->TreeIntsPerHeight), // X descending
		std::vector<uint64_t>(WTSR->TreeSizePadding * WTSR->TreeSizePadding * WTSR->TreeIntsPerHeight), // Z ascending
		std::vector<uint64_t>(WTSR->TreeSizePadding * WTSR->TreeSizePadding * WTSR->TreeIntsPerHeight), // Z descending
	};

	// Face cull the binary columns on the 3 axis, ascending and descending
	faceCullingBinaryColumnsYXZ(columnFaceMasks);

	// Storing planes for all axis, ascending and descending
	std::vector<std::vector<uint64_t>> binaryPlanes{
		std::vector<uint64_t>(WTSR->TreeSizePadding * WTSR->TreeSizePadding * WTSR->TreeIntsPerHeight), // Y ascending
		std::vector<uint64_t>(WTSR->TreeSizePadding * WTSR->TreeSizePadding * WTSR->TreeIntsPerHeight), // Y descending
		std::vector<uint64_t>(WTSR->TreeSizePadding * WTSR->TreeSizePadding * WTSR->TreeIntsPerHeight), // X ascending
		std::vector<uint64_t>(WTSR->TreeSizePadding * WTSR->TreeSizePadding * WTSR->TreeIntsPerHeight), // X descending
		std::vector<uint64_t>(WTSR->TreeSizePadding * WTSR->TreeSizePadding * WTSR->TreeIntsPerHeight), // Z ascending
		std::vector<uint64_t>(WTSR->TreeSizePadding * WTSR->TreeSizePadding * WTSR->TreeIntsPerHeight), // Z descending
	};

	for (int axis = 0; axis < 6; axis++) { // Iterate all axis ascending and descending 
		// Create the binary plane for each axis
		buildBinaryPlanes(columnFaceMasks[axis], binaryPlanes[axis], axis);

		// Greedy mesh each plane and create planes
		greedyMeshingBinaryPlane(binaryPlanes[axis], axis);
	}
}

void ATree::faceCullingBinaryColumnsYXZ(std::vector<std::vector<uint64_t>>& columnFaceMasks) {
	// Face culling for all the 3 axis (Y, X, Z)
	for (int axis = 0; axis < 3; axis++) {
		for (int x = 0; x < WTSR->TreeSizePadding; x++) {
			for (int z = 0; z < WTSR->TreeSizePadding; z++) {
				for (int bitIndex = 0; bitIndex < WTSR->TreeIntsPerHeight; bitIndex++) {
					const int columnIndex{ (x * WTSR->TreeSizePadding * WTSR->TreeIntsPerHeight) + (z * WTSR->TreeIntsPerHeight) + bitIndex };

					uint64_t column = 0;
					switch (axis) {
					case 0:
						column = binaryTree.yBinaryColumn[columnIndex];
						break;
					case 1:
						column = binaryTree.xBinaryColumn[columnIndex];
						break;
					case 2:
						column = binaryTree.zBinaryColumn[columnIndex];
						break;
					}

					// If is the Y axis and not the last bitIndex
					if (axis == 0 && bitIndex < WTSR->TreeIntsPerHeight - 1) {
						const bool isAboveSolid = binaryTree.yBinaryColumn[columnIndex + 1] != 0;
						const bool columnAllSolid = column == std::numeric_limits<uint64_t>::max();

						// Skip creating face between height chunks if there's more solid blocks above 
						if (isAboveSolid && columnAllSolid) {
							continue;
						}
					}

					// Sample ascending axis and set to true when air meets solid
					columnFaceMasks[axis * 2 + 0][columnIndex] = column & ~(column >> 1); // INDEX VERIFIED!

					// Sample descending axis and set to true when air meets solid
					columnFaceMasks[axis * 2 + 1][columnIndex] = column & ~(column << 1); // INDEX VERIFIED!

					// Remove bottom face between height chunk if there are solid blocks underneath
					if (axis == 0) {
						if (bitIndex > 0) {
							const bool isFaceMaskSolid = columnFaceMasks[axis * 2 + 1][columnIndex] != 0;

							// Check if the leftmost bit is 1
							const uint64_t leftmostBitMask = 1ULL << 31;
							const bool isLeftmostBitSet = (binaryTree.yBinaryColumn[columnIndex - 1] & leftmostBitMask) != 0;

							// Remove bottom face if there are solid blocks beneath chunk
							if (isFaceMaskSolid && isLeftmostBitSet) {
								// Flip the rightmost bit to 0
								columnFaceMasks[axis * 2 + 1][columnIndex] &= ~1ULL;
							}
						} else {
							// Remove the bottom face of the world for the bottom chunk
							columnFaceMasks[axis * 2 + 1][columnIndex] &= ~1ULL;
						}
					}

				}
			}
		}
	}
}

void ATree::buildBinaryPlanes(const std::vector<uint64_t>& faceMaskColumn, std::vector<uint64_t>& binaryPlane, const int& axis) {
	for (int x = 0; x < WTSR->TreeSizePadding; x++) {
		for (int z = 0; z < WTSR->TreeSizePadding; z++) {
			for (int bitIndex = 0; bitIndex < WTSR->TreeIntsPerHeight; bitIndex++) {

				const int columnIndex{ (x * WTSR->TreeSizePadding * WTSR->TreeIntsPerHeight) + (z * WTSR->TreeIntsPerHeight) + bitIndex }; // VERIFIED! Goes from 0 - 16,383

				uint64_t column = faceMaskColumn[columnIndex];  // this goes from 0 - 16,383

				// Remove padding only for X and Z axis 
				if (axis != 0 && axis != 1) {
					// Remove the leftmost bit and the rightmost bit and replace them with 0
					column = (column & ~(1ULL << 31)) & ~1ULL;
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
						planeIndex = (y + bitIndex * WTSR->TreeSizePadding) * WTSR->TreeSizePadding + x;
						binaryPlane[planeIndex] |= (1ULL << z);
						break;
					case 2:
					case 3:
					case 4:
					case 5:
						planeRowIndex = y;
						if (columnIndex > 0) {
							currentPlaneIndex = (columnIndex / WTSR->TreeSizePadding) * WTSR->TreeSizePadding;
							planeIndex = currentPlaneIndex + planeRowIndex;
						} else {
							planeIndex = planeRowIndex;
						}

						binaryPlane[planeIndex] |= (1ULL << columnIndex % WTSR->TreeSizePadding);
						break;
					}

					// Remove the padding from the plane
					binaryPlane[planeIndex] = (binaryPlane[planeIndex] & ~(1ULL << 31)) & ~1ULL;

					// Clear the position 
					column &= column - 1;
				}

			}
		}
	}
}

void ATree::greedyMeshingBinaryPlane(std::vector<uint64_t>& planes, const int& axis) {
	for (int row = 0; row < planes.size(); row++) {

		// Removing padding by skipping the first and last row in the plane
		if (row == 0) continue;
		else if (row % WTSR->TreeSizePadding == 0 || row % WTSR->TreeSizePadding == WTSR->TreeSizePadding - 1) continue;

		while (planes[row] != 0) {
			// Get the starting point of the vertex
			const int y = std::countr_zero(planes[row]);

			// Trailing ones are the height of the vertex
			const int height = std::countr_one(planes[row] >> y);

			uint64_t heightMask = ((1ULL << height) - 1) << y;

			// Flip the solid bits used to create the height mask 
			planes[row] = planes[row] & ~heightMask;

			int width = 1;
			int currentPlaneLimit{};

			// Get the expanding limit depending on the axis
			switch (axis) {
			case 0:
			case 1:
				currentPlaneLimit = WTSR->TreeSize; // plane Y max limit is 64

				// Check if the next row can be expanded while in the bounds of the current plane 
				while ((row % WTSR->TreeSizePadding) + width < currentPlaneLimit) {

					// Get the correct row to expand into, depending on the axis 
					const int planesIndex = row + width;

					// Get the bits spanning height for the next row
					const uint64_t nextRowHeight = planes[planesIndex] & heightMask;

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
				currentPlaneLimit = WTSR->TreeHeight * WTSR->TreeSize; // plane X and Z max limit is 64x248  (64 for each layer, and 248 layers)

				// Check if the next row can be expanded while in the bounds of the current plane 
				while (row + (width * WTSR->TreeSizePadding) < currentPlaneLimit) {

					// Get the row above the current one
					const int planesIndex = row + (width * WTSR->TreeSizePadding);

					// Get the bits spanning height for the next row
					const uint64_t nextRowHeight = planes[planesIndex] & heightMask;

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
				voxelX = static_cast<double>(row % WTSR->TreeSizePadding);
				voxelZ = static_cast<double>(y);
				break;
			case 2:
			case 3:
				voxelZ = static_cast<double>(row % WTSR->TreeSizePadding);
				voxelX = static_cast<double>(y);
				break;
			}

			// Height increases with each 64 rows for X and Z
			const double voxelY = row > 0 ? std::floor(static_cast<double>(row / WTSR->TreeSizePadding)) : 0.0;
			voxelPosition1 = { voxelX, voxelZ, voxelY }; // X, Y, Z (height)

			// Modify the original voxel position and create the remaining three quad position
			createAllVoxelPositionsFromOriginal(voxelPosition1, voxelPosition2, voxelPosition3, voxelPosition4, width, height, axis);

			// Create the quads
			createQuadAndAddToMeshData(voxelPosition1, voxelPosition2, voxelPosition3, voxelPosition4, width, height, axis);
		}
	}
}

void ATree::createAllVoxelPositionsFromOriginal(FVector& voxelPosition1, FVector& voxelPosition2, FVector& voxelPosition3, FVector& voxelPosition4, const int& width, const int& height, const int& axis) {
	// Get position modifiers depending on the current axis
	// This values are used to create the 4 quad positions
		// TODO Potentially integrate with the switch above
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

void ATree::createQuadAndAddToMeshData(const FVector& voxelPosition1, const FVector& voxelPosition2, const FVector& voxelPosition3, const FVector& voxelPosition4, const int& height, const int& width, const int& axis) {
	TemporaryMeshData.Vertices.Append({
		voxelPosition1 * WTSR->TreeScale,
		voxelPosition2 * WTSR->TreeScale,
		voxelPosition3 * WTSR->TreeScale,
		voxelPosition4 * WTSR->TreeScale
		});

	// Add triangles and increment vertex count
	TemporaryMeshData.Triangles.Append({
		vertexCount, vertexCount + 1, vertexCount + 2,
		vertexCount + 2, vertexCount + 3, vertexCount
		});

	vertexCount += 4;

	FVector Normal;
	if (axis == 0 || axis == 3 || axis == 5) {
		// Calculate the normals for counter clockwise vectors arrangement
		Normal = FVector::CrossProduct(voxelPosition4 - voxelPosition1, voxelPosition2 - voxelPosition1).GetSafeNormal();
	} else {
		// Calculate the normals for clockwise vectors arrangement
		Normal = FVector::CrossProduct(voxelPosition2 - voxelPosition1, voxelPosition4 - voxelPosition1).GetSafeNormal();
	}

	TemporaryMeshData.Normals.Append({ Normal, Normal, Normal, Normal });

	// Invert the width with the height for the X and Z axis
	if (axis == 0 || axis == 1) {
		TemporaryMeshData.UV0.Append({
			FVector2D(0, 0), FVector2D(0, width), FVector2D(height, width), FVector2D(height, 0)
			});
	} else {
		TemporaryMeshData.UV0.Append({
			FVector2D(0, 0), FVector2D(0, height), FVector2D(width, height), FVector2D(width, 0)
			});
	}

	// TODO Create a dynamic texture and assign a random color from the layer for each 1x1 of the quad. 

	// Assign different random colors for each vertex; This lets the GPU interpolate the colors
	int layerIndex = getColorIndexFromVoxelHeight(voxelPosition1);
	FColor layerColor = WTSR->TreeColorArray[layerIndex];

	TemporaryMeshData.Colors.Append({
		layerColor,
		layerColor,
		layerColor,
		layerColor
		});
}

int ATree::getColorIndexFromVoxelHeight(const FVector& voxelPosition) {
	// TODO Implement this so it chooses a different color
	return 0;
}

void ATree::spawnTreeMeshes() {
	Mesh->CreateMeshSection(0, TemporaryMeshData.Vertices, TemporaryMeshData.Triangles, TemporaryMeshData.Normals, TemporaryMeshData.UV0, TemporaryMeshData.Colors, TArray<FProcMeshTangent>(), hasCollision);

	// Set up simplified collision
	Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	Mesh->SetCollisionResponseToAllChannels(ECR_Block);
	Mesh->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);

	// Load and apply basic material to the mesh
	UMaterialInterface* Material = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/VoxelBasicMaterial.VoxelBasicMaterial"));

	if (Material) {
		Mesh->SetMaterial(0, Material);
	}
}

void ATree::BeginPlay() {
	Super::BeginPlay();


	// Get the basic tree data (branches, trunk height, crown size)

	// Calculate chunks needed to spawn the tree

	// Loop over each chunk and add the voxel data
	createTrunkBinarySolidColumnsYXZ();
	// createCrownkBinarySolidColumnsYXZ();
	createTerrainMeshesData();

	// TODO This should get from a different location, like the binary chunks is done
	// meshData = TemporaryMeshData;

	spawnTreeMeshes();
	UE_LOG(LogTemp, Warning, TEXT("Spawned Tree"));
}
