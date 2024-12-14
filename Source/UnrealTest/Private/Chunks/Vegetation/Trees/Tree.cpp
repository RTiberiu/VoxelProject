

#include "Tree.h"
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

void ATree::createBinarySolidColumnsYXZ() {
	const FVector treeWorldLocation = TreeLocationData.ObjectPosition;

	// Set the tree values to air for all 3 axis (Y, X, Z)
	binaryTree.yBinaryColumn = std::vector<uint32_t>(WTSR->TreeSizePadding * WTSR->TreeSizePadding * WTSR->TreeIntsPerHeight, 0);
	binaryTree.xBinaryColumn = std::vector<uint32_t>(WTSR->TreeSizePadding * WTSR->TreeSizePadding * WTSR->TreeIntsPerHeight, 0);
	binaryTree.zBinaryColumn = std::vector<uint32_t>(WTSR->TreeSizePadding * WTSR->TreeSizePadding * WTSR->TreeIntsPerHeight, 0);

	// Loop over the tree dimensions (X, Y, Z)
	const int halfSize{ WTSR->TreeSizePadding / 2 };
	const int threshold{ 4 };

	// TODO Move this random generator to a different class, as this should be initialized only once
	// Create a random number generator
	std::random_device rd;  
	std::mt19937 gen(rd());

	// Define the distribution
	std::uniform_int_distribution<> treeTrunkDistribution(10, WTSR->TreeHeight);
	std::uniform_int_distribution<> treeBranchDistribution(0, 4);
	std::uniform_int_distribution<> treeBranchLengthDistribution(10, 40);

	// Get the tree trunk voxels and add them to the binary columns
	const int treeTrunkHeight = treeTrunkDistribution(gen);
	const int treeBranches = treeBranchDistribution(gen);

	for (int x = 0; x < WTSR->TreeSizePadding; x++) {

		bool insideThresholdX = x < halfSize + threshold && x > halfSize - threshold;

		if (!insideThresholdX) continue;

		for (int z = 0; z < WTSR->TreeSizePadding; z++) {

			bool insideThresholdZ = z < halfSize + threshold && z > halfSize - threshold;

			if (!insideThresholdZ) continue;

			// Ensuring height remains between chunk borders
			uint32_t height = std::clamp(treeTrunkHeight, 0, static_cast<int>(WTSR->TreeHeight));

			// Add enough bits to y to cover the entire height (4 32bit integers when the max height is 128)
			for (int bitIndex = 0; bitIndex < WTSR->TreeIntsPerHeight; bitIndex++) {
				uint32_t yHeight;

				if (height >= 32) {
					yHeight = ~0U; // Set all bits to 1
					height -= 32;
				} else {
					yHeight = (1U << height) - 1; // Set bits = height
					height = 0;
				}

				// Get index of y 
				const int yIndex{ (x * WTSR->TreeSizePadding * WTSR->TreeIntsPerHeight) + (z * WTSR->TreeIntsPerHeight) + bitIndex };

				// Add blocks height data (Y) to the current X and Z
				binaryTree.yBinaryColumn[yIndex] = yHeight;

				// Skip iteration if Y column is all air
				if (binaryTree.yBinaryColumn[yIndex] == 0) {
					continue;
				}

				for (int y = 0; y < WTSR->TreeSizePadding; y++) {
					// Apply 3D noise to the Y column
					// apply3DNoiseToHeightColumn(binaryTree.yBinaryColumn[yIndex], x, z, y, bitIndex, treeWorldLocation, height);

					const uint32_t currentYCol = binaryTree.yBinaryColumn[yIndex];

					// Next Y index (column) means the same X index (column), but a change in Y bit index
					const int xIndex{ (y * WTSR->TreeSizePadding) + (bitIndex * WTSR->TreeSizePadding * WTSR->TreeSizePadding) + x };

					// y'th bit of column Y
					const uint8_t nthBitY = (currentYCol >> y) & 1;

					// Create temporary variable for column X
					uint32_t xBitTemp = binaryTree.xBinaryColumn[xIndex];

					// Apply the change to the temporary X bit
					xBitTemp = (xBitTemp >> z) | nthBitY;

					// Assign to actual bit the change
					binaryTree.xBinaryColumn[xIndex] = (xBitTemp << z) | binaryTree.xBinaryColumn[xIndex];

					// Next Y index (column) means the next Z index (column), but the same Y bit index
					const int zIndex{ (y * WTSR->TreeSizePadding) + (bitIndex * WTSR->TreeSizePadding * WTSR->TreeSizePadding) + z };

					// Create temporary variable for column Z
					uint32_t zBitTemp = binaryTree.zBinaryColumn[zIndex];

					// Apply the change to the temporary Z bit
					zBitTemp = (zBitTemp >> x) | nthBitY;

					// Assign to actual bit the change
					binaryTree.zBinaryColumn[zIndex] = (zBitTemp << x) | binaryTree.zBinaryColumn[zIndex];
				}
			}

		}
	}
}

void ATree::apply3DNoiseToHeightColumn(uint32_t& column, int& x, int& z, int& y, int& bitIndex, const FVector& treeWorldLocation, int& height) {
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
		column |= 1U << y;
	}
}

void ATree::createTerrainMeshesData() {
	// Storing the face masks for the Y, X, Z axis
	// Size is doubled to contains both ascending and descending columns 
	std::vector<std::vector<uint32_t>> columnFaceMasks{
		std::vector<uint32_t>(WTSR->TreeSizePadding * WTSR->TreeSizePadding * WTSR->TreeIntsPerHeight), // Y ascending
		std::vector<uint32_t>(WTSR->TreeSizePadding * WTSR->TreeSizePadding * WTSR->TreeIntsPerHeight), // Y descending
		std::vector<uint32_t>(WTSR->TreeSizePadding * WTSR->TreeSizePadding * WTSR->TreeIntsPerHeight), // X ascending
		std::vector<uint32_t>(WTSR->TreeSizePadding * WTSR->TreeSizePadding * WTSR->TreeIntsPerHeight), // X descending
		std::vector<uint32_t>(WTSR->TreeSizePadding * WTSR->TreeSizePadding * WTSR->TreeIntsPerHeight), // Z ascending
		std::vector<uint32_t>(WTSR->TreeSizePadding * WTSR->TreeSizePadding * WTSR->TreeIntsPerHeight), // Z descending
	};

	// Face cull the binary columns on the 3 axis, ascending and descending
	faceCullingBinaryColumnsYXZ(columnFaceMasks);

	// Storing planes for all axis, ascending and descending
	std::vector<std::vector<uint32_t>> binaryPlanes{
		std::vector<uint32_t>(WTSR->TreeSizePadding * WTSR->TreeSizePadding * WTSR->TreeIntsPerHeight), // Y ascending
		std::vector<uint32_t>(WTSR->TreeSizePadding * WTSR->TreeSizePadding * WTSR->TreeIntsPerHeight), // Y descending
		std::vector<uint32_t>(WTSR->TreeSizePadding * WTSR->TreeSizePadding * WTSR->TreeIntsPerHeight), // X ascending
		std::vector<uint32_t>(WTSR->TreeSizePadding * WTSR->TreeSizePadding * WTSR->TreeIntsPerHeight), // X descending
		std::vector<uint32_t>(WTSR->TreeSizePadding * WTSR->TreeSizePadding * WTSR->TreeIntsPerHeight), // Z ascending
		std::vector<uint32_t>(WTSR->TreeSizePadding * WTSR->TreeSizePadding * WTSR->TreeIntsPerHeight), // Z descending
	};

	for (int axis = 0; axis < 6; axis++) { // Iterate all axis ascending and descending 
		// Create the binary plane for each axis
		buildBinaryPlanes(columnFaceMasks[axis], binaryPlanes[axis], axis);

		// Greedy mesh each plane and create planes
		greedyMeshingBinaryPlane(binaryPlanes[axis], axis);
	}
}

void ATree::faceCullingBinaryColumnsYXZ(std::vector<std::vector<uint32_t>>& columnFaceMasks) {
	// Face culling for all the 3 axis (Y, X, Z)
	for (int axis = 0; axis < 3; axis++) {
		for (int x = 0; x < WTSR->TreeSizePadding; x++) {
			for (int z = 0; z < WTSR->TreeSizePadding; z++) {
				for (int bitIndex = 0; bitIndex < WTSR->TreeIntsPerHeight; bitIndex++) {
					const int columnIndex{ (x * WTSR->TreeSizePadding * WTSR->TreeIntsPerHeight) + (z * WTSR->TreeIntsPerHeight) + bitIndex };

					uint32_t column = 0;
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
						const bool columnAllSolid = column == std::numeric_limits<uint32_t>::max();

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
							const uint32_t leftmostBitMask = 1U << 31;
							const bool isLeftmostBitSet = (binaryTree.yBinaryColumn[columnIndex - 1] & leftmostBitMask) != 0;

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

void ATree::buildBinaryPlanes(const std::vector<uint32_t>& faceMaskColumn, std::vector<uint32_t>& binaryPlane, const int& axis) {
	for (int x = 0; x < WTSR->TreeSizePadding; x++) {
		for (int z = 0; z < WTSR->TreeSizePadding; z++) {
			for (int bitIndex = 0; bitIndex < WTSR->TreeIntsPerHeight; bitIndex++) {

				const int columnIndex{ (x * WTSR->TreeSizePadding * WTSR->TreeIntsPerHeight) + (z * WTSR->TreeIntsPerHeight) + bitIndex }; // VERIFIED! Goes from 0 - 16,383

				uint32_t column = faceMaskColumn[columnIndex];  // this goes from 0 - 16,383

				// Remove padding only for X and Z axis 
				if (axis != 0 && axis != 1) {
					// Remove the leftmost bit and the rightmost bit and replace them with 0
					column = (column & ~(1U << 31)) & ~1U;
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
						binaryPlane[planeIndex] |= (1U << z);
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

						binaryPlane[planeIndex] |= (1U << columnIndex % WTSR->TreeSizePadding);
						break;
					}

					// Remove the padding from the plane
					binaryPlane[planeIndex] = (binaryPlane[planeIndex] & ~(1U << 31)) & ~1U;

					// Clear the position 
					column &= column - 1;
				}

			}
		}
	}
}

void ATree::greedyMeshingBinaryPlane(std::vector<uint32_t>& planes, const int& axis) {
	for (int row = 0; row < planes.size(); row++) {

		// Removing padding by skipping the first and last row in the plane
		if (row == 0) continue;
		else if (row % WTSR->TreeSizePadding == 0 || row % WTSR->TreeSizePadding == WTSR->TreeSizePadding - 1) continue;

		while (planes[row] != 0) {
			// Get the starting point of the vertex
			const int y = std::countr_zero(planes[row]);

			// Trailing ones are the height of the vertex
			const int height = std::countr_one(planes[row] >> y);

			uint32_t heightMask = ((1U << height) - 1) << y;

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
					const uint32_t nextRowHeight = planes[planesIndex] & heightMask;

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
					const uint32_t nextRowHeight = planes[planesIndex] & heightMask;

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

	createBinarySolidColumnsYXZ();
	createTerrainMeshesData();

	// TODO This should get from a different location, like the binary chunks is done
	// meshData = TemporaryMeshData;

	spawnTreeMeshes();
	UE_LOG(LogTemp, Warning, TEXT("Spawned Tree"));
}
