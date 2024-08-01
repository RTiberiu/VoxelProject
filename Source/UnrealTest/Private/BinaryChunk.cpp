// Fill out your copyright notice in the Description page of Project Settings.

#include "BinaryChunk.h"
#include "ProceduralMeshComponent.h"

#include <iostream>
#include <array>
#include <bitset>
#include <limits>
#include "FastNoiseLite.h"


// Sets default values
ABinaryChunk::ABinaryChunk() {
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	Mesh = CreateDefaultSubobject<UProceduralMeshComponent>("Mesh");

	// Declaring the noise and initializing settings
	noise = new FastNoiseLite();
	noise->SetNoiseType(FastNoiseLite::NoiseType_Perlin);
	noise->SetFractalType(FastNoiseLite::FractalType_FBm);

	Mesh->SetCastShadow(false);

	// Set mesh as root
	SetRootComponent(Mesh);
}

void ABinaryChunk::SetWorldTerrainSettings(UWorldTerrainSettings* InWorldTerrainSettings) {
	WorldTerrainSettingsRef = InWorldTerrainSettings;
}

// Debugging function that prints a 64-bit integer in groups
void ABinaryChunk::printBinary(uint64_t value, int groupSize, const std::string& otherData) {
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

void ABinaryChunk::createBinarySolidColumnsYXZ() {
	// Get current chunk world position
	const FVector chunkWorldLocation = GetActorLocation();

	constexpr std::array<float, 3> octaveFrequencies{ 0.02f, 0.025f, 0.03f };

	// Set the chunk values to air for all 3 axis (Y, X, Z)
	binaryChunk.yBinaryColumn = std::vector<uint64_t>(chunkSizePadding * chunkSizePadding * intsPerHeight, 0);
	binaryChunk.xBinaryColumn = std::vector<uint64_t>(chunkSizePadding * chunkSizePadding * intsPerHeight, 0);
	binaryChunk.zBinaryColumn = std::vector<uint64_t>(chunkSizePadding * chunkSizePadding * intsPerHeight, 0);

	// Loop over the chunk dimensions (X, Y, Z)
	for (int x = 0; x < chunkSizePadding; x++) {
		for (int z = 0; z < chunkSizePadding; z++) {

			// Looping over the different octaves to get the final height
			int amplitude{ 0 };
			int height{ 0 };
			for (const float octave : octaveFrequencies) {
				// Set frequency and get value between 0 and 2
				noise->SetFrequency(octave);
				// Getting perlin noise position, adjusted to the Unreal Engine grid system 
				const float noisePositionX = static_cast<float>((x * 100 + chunkWorldLocation.X) / 100);
				const float noisePositionZ = static_cast<float>((z * 100 + chunkWorldLocation.Y) / 100);
				const float noiseValue = noise->GetNoise(noisePositionX, noisePositionZ) + 1;

				// Adding multiple splines to the perlinValue
				if (noiseValue <= 1) {
					amplitude = 15;
				} else if (noiseValue <= 1.4) {
					amplitude = 20;
				} else if (noiseValue <= 1.8) {
					amplitude = 30;
				} else if (noiseValue <= 2) {
					amplitude = 25;
				}

				// Multiply noise by amplitude and reduce to integer
				height += static_cast<int>(std::floor(noiseValue * amplitude));
			}

			// Ensuring height remains between chunk borders
			height = std::clamp(height, 0, static_cast<int>(chunkHeight));

			// Add enough bits to y to cover the entire height (4 64bit integers when the max height is 256)
			for (int bitIndex = 0; bitIndex < intsPerHeight; bitIndex++) {
				uint64_t yHeight;

				if (height >= 64) {
					yHeight = ~0ULL; // Set all bits to 1
					height -= 64;
				} else {
					yHeight = (1ULL << height) - 1; // Set bits = height
					height = 0;
				}

				// Get index of y 
				int yIndex{ (x * chunkSizePadding * intsPerHeight) + (z * intsPerHeight) + bitIndex };

				// Add blocks height data (Y) to the current X and Z
				binaryChunk.yBinaryColumn[yIndex] = yHeight;
				uint64_t currentYCol = binaryChunk.yBinaryColumn[yIndex]; // TODO MAYBE COMBINE THIS TWO LINES (the one above)

				// Skip iteration if Y column is all air
				if (currentYCol == 0) {
					continue;
				}

				for (int y = 0; y < chunkSizePadding; y++) {
					// Next Y index (column) means the same X index (column), but a change in Y bit index
					int xIndex{ (y * chunkSizePadding) + (bitIndex * chunkSizePadding * chunkSizePadding) + x };

					// y'th bit of column Y
					uint8_t nthBitY = (currentYCol >> y) & 1;

					// Create temporary variable for column X
					uint64_t xBitTemp = binaryChunk.xBinaryColumn[xIndex];

					// Apply the change to the temporary X bit
					xBitTemp = (xBitTemp >> z) | nthBitY;

					// Assign to actual bit the change
					binaryChunk.xBinaryColumn[xIndex] = (xBitTemp << z) | binaryChunk.xBinaryColumn[xIndex];

					// Next Y index (column) means the next Z index (column), but the same Y bit index
					int zIndex{ (y * chunkSizePadding) + (bitIndex * chunkSizePadding * chunkSizePadding) + z }; // FUCKING VERIFIED

					// Create temporary variable for column Z
					uint64_t zBitTemp = binaryChunk.zBinaryColumn[zIndex];

					// Apply the change to the temporary Z bit
					zBitTemp = (zBitTemp >> x) | nthBitY;

					// Assign to actual bit the change
					binaryChunk.zBinaryColumn[zIndex] = (zBitTemp << x) | binaryChunk.zBinaryColumn[zIndex];

				}
			}

		}
	}
}

void ABinaryChunk::faceCullingBinaryColumnsYXZ(std::vector<std::vector<uint64_t>>& columnFaceMasks) {
	// Face culling for all the 3 axis (Y, X, Z)
	for (int axis = 0; axis < 3; axis++) {
		for (int x = 0; x < chunkSizePadding; x++) {
			for (int z = 0; z < chunkSizePadding; z++) {
				for (int bitIndex = 0; bitIndex < intsPerHeight; bitIndex++) {
					int columnIndex{ (x * chunkSizePadding * intsPerHeight) + (z * intsPerHeight) + bitIndex };

					uint64_t column = 0;
					switch (axis) {
					case 0:
						column = binaryChunk.yBinaryColumn[columnIndex];
						break;
					case 1:
						column = binaryChunk.xBinaryColumn[columnIndex];
						break;
					case 2:
						column = binaryChunk.zBinaryColumn[columnIndex];
						break;
					}

					// If is the Y axis and not the last bitIndex
					if (axis == 0 && bitIndex < intsPerHeight - 1) {
						bool isAboveSolid = binaryChunk.yBinaryColumn[columnIndex + 1] != 0;
						bool columnAllSolid = column == std::numeric_limits<uint64_t>::max();

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
							bool isFaceMaskSolid = columnFaceMasks[axis * 2 + 1][columnIndex] != 0;

							// Check if the leftmost bit is 1
							uint64_t leftmostBitMask = 1ULL << 63;
							bool isLeftmostBitSet = (binaryChunk.yBinaryColumn[columnIndex - 1] & leftmostBitMask) != 0;
						
							// Remove bottom face if there are solid blocks beneath chunk
							if (isFaceMaskSolid && isLeftmostBitSet) {
								// Flip the rightmost bit to 0
								uint64_t rightmostBitMask = ~1ULL;
								columnFaceMasks[axis * 2 + 1][columnIndex] &= rightmostBitMask;
							}
						} else {
							// Remove the bottom face of the world for the bottom chunk
							uint64_t rightmostBitMask = ~1ULL;
							columnFaceMasks[axis * 2 + 1][columnIndex] &= rightmostBitMask;
						}
					}

				}
			}
		}
	}
}

// Calculate the starting position of a voxel, depending on the current column and axis 
FVector ABinaryChunk::getVoxelStartingPosition(const int& height, const int& axis, const int& x, const int& z, const int& bitIndex, const int& columnIndex) {
	FVector voxelPosition1(3);
	switch (axis) {
	case 0:
	case 1: {
		// Get y's starting position, depending on the current bitIndex
		int startingPositionY = bitIndex * chunkSizePadding;
		//									  X                      Y                     Z
		voxelPosition1 = { static_cast<float>(x), static_cast<float>(z), static_cast<float>(height + startingPositionY) }; // up / down 
		break;
	}
	case 2:
	case 3: {
		int currentPositionX = columnIndex > 0 ? columnIndex / chunkSizePadding : 0;

		// Ensures X goes to 64 and resets to 0 every time; this is done because
		// there is the bitIndex inner loop that increments 4 with every Z
		int xAltered{ ((z * intsPerHeight) % chunkSizePadding) + bitIndex };

		voxelPosition1 = { static_cast<float>(xAltered), static_cast<float>(height), static_cast<float>(currentPositionX) }; // right / left 
		break;
	}
	case 4:
	case 5: {
		int currentPositionZ = columnIndex > 0 ? columnIndex / chunkSizePadding : 0;

		int zAltered{ ((z * intsPerHeight) % chunkSizePadding) + bitIndex };

		voxelPosition1 = { static_cast<float>(height), static_cast<float>(zAltered), static_cast<float>(currentPositionZ) }; // forward / backwards 
		break;
	}
	default:
		UE_LOG(LogTemp, Error, TEXT("Invalid axis value: %d"), axis);
		ensureMsgf(false, TEXT("Unhandled case in switch statement for axis: %d"), axis);
		break;
	}

	return voxelPosition1;
}

// Modify the original voxel position and create the remaining three quad position
void ABinaryChunk::createAllVoxelPositionsFromOriginal(
	FVector& voxelPosition1, 
	FVector& voxelPosition2, 
	FVector& voxelPosition3, 
	FVector& voxelPosition4, 
	const int& width, const int& height, const int& axis) {

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
		widthPositionModifier[0] = width; // NOT SURE IF IT'S WIDTH HERE
		heightPositionModifier[1] = height; // NOT SURE IF IT'S HEIGHT HERE

		// Adjust face direction
		voxelPosition1 += bottomFace;

		voxelPosition2 = voxelPosition1 + heightPositionModifier; // bottom - right
		voxelPosition3 = voxelPosition1 + widthPositionModifier + heightPositionModifier; // top - right
		voxelPosition4 = voxelPosition1 + widthPositionModifier;// top - left
		break;
	case 1: // Y axis descending
		widthPositionModifier[0] = width; // NOT SURE IF IT'S WIDTH HERE
		heightPositionModifier[1] = height; // NOT SURE IF IT'S HEIGHT HERE

		voxelPosition2 = voxelPosition1 + widthPositionModifier; // top - left
		voxelPosition3 = voxelPosition1 + widthPositionModifier + heightPositionModifier; // top - right
		voxelPosition4 = voxelPosition1 + heightPositionModifier; // bottom - right
		break;
	case 2: // X axis ascending
		widthPositionModifier[0] = width; // NOT SURE IF IT'S WIDTH HERE
		heightPositionModifier[2] = height; // NOT SURE IF IT'S HEIGHT HERE

		// Adjust face direction
		voxelPosition1 += backwardFace;

		voxelPosition2 = voxelPosition1 + widthPositionModifier; // top - left
		voxelPosition3 = voxelPosition1 + widthPositionModifier + heightPositionModifier; // top - right
		voxelPosition4 = voxelPosition1 + heightPositionModifier; // bottom - right
		break;
	case 3: // X axis descending
		widthPositionModifier[0] = width; // NOT SURE IF IT'S WIDTH HERE
		heightPositionModifier[2] = height; // NOT SURE IF IT'S HEIGHT HERE

		voxelPosition2 = voxelPosition1 + heightPositionModifier; // bottom - right
		voxelPosition3 = voxelPosition1 + widthPositionModifier + heightPositionModifier; // top - right
		voxelPosition4 = voxelPosition1 + widthPositionModifier;// top - left
		break;
	case 4:	// Z axis ascending
		widthPositionModifier[1] = width; // NOT SURE IF IT'S WIDTH HERE
		heightPositionModifier[2] = height; // NOT SURE IF IT'S HEIGHT HERE

		// Adjust face direction
		voxelPosition1 += rightFace;

		voxelPosition2 = voxelPosition1 + heightPositionModifier; // top - left
		voxelPosition3 = voxelPosition1 + widthPositionModifier + heightPositionModifier; // top - right
		voxelPosition4 = voxelPosition1 + widthPositionModifier; // bottom - right
		break;
	case 5: // Z axis descending
		widthPositionModifier[1] = width; // NOT SURE IF IT'S WIDTH HERE
		heightPositionModifier[2] = height; // NOT SURE IF IT'S HEIGHT HERE

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


void ABinaryChunk::createTerrainMeshesData() {
	// Storing the face masks for the Y, X, Z axis
	// Size is doubled to contains both ascending and descending columns 
	std::vector<std::vector<uint64_t>> columnFaceMasks{
		std::vector<uint64_t>(chunkSizePadding * chunkSizePadding * intsPerHeight), // Y ascending
		std::vector<uint64_t>(chunkSizePadding * chunkSizePadding * intsPerHeight), // Y descending
		std::vector<uint64_t>(chunkSizePadding * chunkSizePadding * intsPerHeight), // X ascending
		std::vector<uint64_t>(chunkSizePadding * chunkSizePadding * intsPerHeight), // X descending
		std::vector<uint64_t>(chunkSizePadding * chunkSizePadding * intsPerHeight), // Z ascending
		std::vector<uint64_t>(chunkSizePadding * chunkSizePadding * intsPerHeight), // Z descending
	};

	// Face cull the binary columns on the 3 axis, ascending and descending
	faceCullingBinaryColumnsYXZ(columnFaceMasks);

	// Storing planes for all axis, ascending and descending
	std::vector<std::vector<uint64_t>> binaryPlanes{ 
		std::vector<uint64_t>(chunkSizePadding * chunkSizePadding * intsPerHeight), // Y ascending
		std::vector<uint64_t>(chunkSizePadding * chunkSizePadding * intsPerHeight), // Y descending
		std::vector<uint64_t>(chunkSizePadding * chunkSizePadding * intsPerHeight), // X ascending
		std::vector<uint64_t>(chunkSizePadding * chunkSizePadding * intsPerHeight), // X descending
		std::vector<uint64_t>(chunkSizePadding * chunkSizePadding * intsPerHeight), // Z ascending
		std::vector<uint64_t>(chunkSizePadding * chunkSizePadding * intsPerHeight), // Z descending
	};

	for (int axis = 0; axis < 6; axis++) { // Iterate all axis ascending and descending // 6 value

		// Create the binary plane for each axis
		buildBinaryPlanes(columnFaceMasks[axis], binaryPlanes[axis], axis);

		//  PRINTING THE PLANES FOR TESTING 
		//UE_LOG(LogTemp, Warning, TEXT("-------------- Planes for axis %d --------------"), axis);
		//for (int row = 2880; row < 3072; row++) { // for (int row = 0; row < binaryPlanes[axis].size(); row++) 

		//	if (row == 0) {
		//		UE_LOG(LogTemp, Warning, TEXT("--- Plane %d --- "), 1);
		//	} else if (row % chunkSizePadding == 0) {
		//		UE_LOG(LogTemp, Warning, TEXT("\n\n--- Plane %d Row: %d  --- "), row / chunkSizePadding, row);
		//	}

		//	const std::string message = "Row " + std::to_string(row) + ": ";

		//	printBinary(binaryPlanes[axis][row], 8, message);
		//}

		// PRINTING THE 3 LAYERS ONE AT THE TIME 
		/*UE_LOG(LogTemp, Warning, TEXT("-------------- Slices of Z axis --------------"), axis);
		int counter{ 0 };
		for (int row = 2880; row < 2944; row++) { 
			const std::string message = "Row " + std::to_string(counter) + ": ";

			printBinary(binaryPlanes[axis][row], 8, message);
			printBinary(binaryPlanes[axis][row + 64], 8, message);
			printBinary(binaryPlanes[axis][row + 64 + 64], 8, message);

			UE_LOG(LogTemp, Warning, TEXT("\n----------------------------\n"));

			counter++;
		}*/

		// Greedy mesh each plane and create planes
		greedyMeshingBinaryPlane(binaryPlanes[axis], axis);
	}

	

	// --------------------------------------------------
	//  Below is the old implementation without greedy meshing (that still works)
	// --------------------------------------------------

	// Find faces and build binary planes based on the voxel block
	//for (int axis = 0; axis < 6; axis++) { // Iterate all axis ascending and descending // 6 value
	//	for (int x = 0; x < chunkSizePadding; x++) {
	//		for (int z = 0; z < chunkSizePadding; z++) {
	//			for (int bitIndex = 0; bitIndex < intsPerHeight; bitIndex++) {

	//				int columnIndex{ (x * chunkSizePadding * intsPerHeight) + (z * intsPerHeight) + bitIndex }; // VERIFIED! Goes from 0 - 16,383

	//				uint64_t column = columnFaceMasks[axis][columnIndex];  // this goes from 0 - 16,383

	//				// Remove padding only for X and Z axis 
	//				if (axis != 0 && axis != 1) {
	//					// Remove the leftmost bit and the rightmost bit and replace them with 0
	//					column = (column & ~(1ULL << 63)) & ~1ULL;
	//				}

	//				while (column != 0) {
	//					// Get the trailing zeros for the current column
	//					int y = std::countr_zero(column);

	//					// Clear the position 
	//					column &= column - 1;

	//					FVector voxelPosition1 = getVoxelStartingPosition(y, axis, x, z, bitIndex, columnIndex);
	//					

	//					// TODO Add greedy meshing and get the height and width 
	//					
	//					int width{ 1 }; // TODO change after greeding meshing
	//					int height{ 1 }; // TODO change after greeding meshing

	//					FVector voxelPosition2(3);
	//					FVector voxelPosition3(3);
	//					FVector voxelPosition4(3);

	//					// Modify the original voxel position and create the remaining three quad position
	//					createAllVoxelPositionsFromOriginal(voxelPosition1, voxelPosition2, voxelPosition3, voxelPosition4, width, height, axis);

	//					// Create the quads
	//					createQuadAndAddToMeshData(voxelPosition1, voxelPosition2, voxelPosition3, voxelPosition4, width, height, axis);

	//				}
	//			}
	//		}
	//	}
	//}
}

void ABinaryChunk::buildBinaryPlanes(const std::vector<uint64_t>& faceMaskColumn, std::vector<uint64_t>& binaryPlane, const int& axis) {
	for (int x = 0; x < chunkSizePadding; x++) {
		for (int z = 0; z < chunkSizePadding; z++) {
			for (int bitIndex = 0; bitIndex < intsPerHeight; bitIndex++) {

				int columnIndex{ (x * chunkSizePadding * intsPerHeight) + (z * intsPerHeight) + bitIndex }; // VERIFIED! Goes from 0 - 16,383
			
				uint64_t column = faceMaskColumn[columnIndex];  // this goes from 0 - 16,383

				// Remove padding only for Y axis 
				if (axis != 0 && axis != 1) {
					// Remove the leftmost bit and the rightmost bit and replace them with 0
					column = (column & ~(1ULL << 63)) & ~1ULL;
				}

				while (column != 0) {
					// Get the trailing zeros for the current column
					int y = std::countr_zero(column);
					int planeIndex;
					int currentPlaneIndex;
					int planeRowIndex;
					switch (axis) {
					case 0:
					case 1:
						// Get to the correct plane and then add x to get to the correct row in the plane
						planeIndex = planeIndex = (y + bitIndex * chunkSizePadding) * chunkSizePadding + x; // THIS IS CORRECT NOW! 

						binaryPlane[planeIndex] |= (1ULL << z); // VERIFIED!
						break;
					case 2:
					case 3:
					
						planeRowIndex = y; 
						if (columnIndex > 0) {
							currentPlaneIndex = (columnIndex / chunkSizePadding) * chunkSizePadding;

							planeIndex = currentPlaneIndex + planeRowIndex;
						} else {
							planeIndex = planeRowIndex;
						}

						binaryPlane[planeIndex] |= (1ULL << columnIndex % chunkSizePadding); 
						
						break;
					case 4:
					case 5: // WORK IN PROGRESS
						// planeRowIndex = (chunkSizePadding - y - 1);
						planeRowIndex = y; 
						if (columnIndex > 0) {
							currentPlaneIndex = (columnIndex / chunkSizePadding) * chunkSizePadding;

							planeIndex = currentPlaneIndex + planeRowIndex;
						} else {
							planeIndex = planeRowIndex;
						}

						binaryPlane[planeIndex] |= (1ULL << columnIndex % chunkSizePadding);
						break;
					}

					// Clear the position 
					column &= column - 1;

				}
			
			}
		}
	}
}

//// THIS IS ALL WORK IN PROGRESS. I HAVE TO VALIDATE THE ENTIRE LOGIC AND CONTINUE.
//// The idea is to traverse each plane, expand and get the points for the planes for the current plane
//// and then create the vertices with the current height and width
////
//// I also still have to validate the planes value. 
//void ABinaryChunk::greedyMeshingBinaryPlane(std::vector<uint64_t>& planes, const int& axis) {
//	// Cycle through the plane
//	for (int x = 0; x < chunkSizePadding; x++) {
//		for (int z = 0; z < chunkSizePadding; z++) {
//			for (int bitIndex = 0; bitIndex < intsPerHeight; bitIndex++) {
//
//				int row{ (x * chunkSizePadding * intsPerHeight) + (z * intsPerHeight) + bitIndex };
//
//				// I think I still have to create the planes, otherwise I have to go row by row to expand
//				// The idea is that I already face culled the binary columns, so my height will always be 1 in that case
//				// That's why I need to create a plane, to get the "true" height. 
//				// That "true" height is actually multiple columns that share the same height. 
//				// After getting the height, I can expand to the next row and try to expand again width wise.
//				// Moving to the next row in reality means try to expand the same columns but on a different row. 
//				
//
//				// Create vertices up until there are no more to create in the current row (this allows 3D terrain)
//				while (planes[row] != 0) {
//					// Get the starting point of the vertex
//					int y = std::countr_zero(planes[row]);
//
//					// Trailing ones are the height of the vertex
//					int height = std::countr_one(planes[row] >> y);
//
//					uint64_t heightMask = ((1ULL << height) - 1) << y;
//
//					// Flip the solid bits used to create the height mask 
//					planes[row] = planes[row] & ~heightMask;
//
//					int width = 1;
//					int currentPlaneIndex{ 0 };
//					const uint16_t* currentPlaneLimit{};
//
//					// Get the expanding limit depending on the axis
//					switch (axis) {
//					case 0:
//					case 1:
//						currentPlaneIndex = row % chunkSizePadding; // plane for Y goes from 0 to 64 
//						currentPlaneLimit = &chunkSizePadding; // plane Y max limit is 64
//						break;
//					case 2:
//					case 3:
//					case 4:
//					case 5:
//						currentPlaneIndex = row % chunkHeight; // plane for X and Z goes from 0 to 248
//						currentPlaneLimit = &chunkHeight; // plane X and Z max limit is 248
//						break;
//					}
//
//					// Check if the next row can be expanded while in the bounds of the current plane 
//					while (currentPlaneIndex + width < *currentPlaneLimit) {
//						// Get the bits spanning height for the next row
//						int nextRowHeight = planes[row + width + intsPerHeight] & heightMask;
//
//						if (nextRowHeight != heightMask) {
//							break; // Can't expand horizontally
//						}
//
//						// Remove the bits we expanded into
//						planes[row + width + intsPerHeight] = planes[row + width + intsPerHeight] & ~heightMask;
//
//						width++;
//					}
//
//					// Create quad from 
//					// FVector voxelPosition1 = getVoxelStartingPosition(y, axis, x, z, bitIndex, row); // old version
//					FVector voxelPosition1 = getVoxelStartingPosition(y, axis, x, z, bitIndex, row);
//
//					// int width{ 1 }; // TODO change after greeding meshing
//					// int height{ 1 }; // TODO change after greeding meshing
//
//					FVector voxelPosition2(3);
//					FVector voxelPosition3(3);
//					FVector voxelPosition4(3);
//
//					// Modify the original voxel position and create the remaining three quad position
//					createAllVoxelPositionsFromOriginal(voxelPosition1, voxelPosition2, voxelPosition3, voxelPosition4, width, height, axis);
//
//					// Create the quads
//					createQuadAndAddToMeshData(voxelPosition1, voxelPosition2, voxelPosition3, voxelPosition4, width, height, axis);
//				}
//			}
//		}
//	}
//}

void ABinaryChunk::greedyMeshingBinaryPlane(std::vector<uint64_t>& planes, const int& axis) {
	for (int row = 0; row < planes.size(); row++) { // int row = 2880; row < 2907; // int row = 0; row < planes.size();

		//UE_LOG(LogTemp, Warning, TEXT("--- NEXT ROW ---"));
		while (planes[row] != 0) {
			// Get the starting point of the vertex
			int y = std::countr_zero(planes[row]);

			// Trailing ones are the height of the vertex
			int height = std::countr_one(planes[row] >> y);

			//printBinary(planes[row], 8, "Row: " + std::to_string(row) + " Planes row : ");

			uint64_t heightMask = ((1ULL << height) - 1) << y;

			//printBinary(heightMask, 8, "Height mask: ");

			// Flip the solid bits used to create the height mask 
			planes[row] = planes[row] & ~heightMask;

			//printBinary(planes[row], 8, "Planes row after flipping with height mask: ");

			int width = 1;
			int currentPlaneLimit{};

			// Get the expanding limit depending on the axis
			switch (axis) {
			case 0:
			case 1: // TODO There is no greedy meshing for the height/width, not sure which one 
				currentPlaneLimit = chunkSizePadding; // plane Y max limit is 64

				// Check if the next row can be expanded while in the bounds of the current plane 
				while ((row % chunkSizePadding) + width < currentPlaneLimit) {

					// Get the correct row to expand into, depending on the axis 
					int planesIndex = row + width;

					// Get the bits spanning height for the next row
					uint64_t nextRowHeight = planes[planesIndex] & heightMask;

					//UE_LOG(LogTemp, Warning, TEXT("nextRowHeight: "));
					//printBinary(nextRowHeight, 8);

					if (nextRowHeight != heightMask) {
						break; // Can't expand horizontally
					}

					// Remove the bits we expanded into
					planes[planesIndex] = planes[planesIndex] & ~heightMask;

					//UE_LOG(LogTemp, Warning, TEXT("Incremented width for axis %d"), axis);

					width++;
				}
				break;
			case 2:
			case 3: // THIS PART SHOULD BE FINE. If there is an issue, it might be below when creating the vertices
			case 4:
			case 5:
				currentPlaneLimit = chunkHeight * chunkSizePadding; // plane X and Z max limit is 64x248  (64 for each layer, and 248 layers)

				//UE_LOG(LogTemp, Warning, TEXT("\tPlane limit: %d\tValue to be lower than plane limit: %d\tNext row index: %d"), currentPlaneLimit, row + (width * chunkSizePadding), row + (width * chunkSizePadding));

				// Check if the next row can be expanded while in the bounds of the current plane 
				while (row + (width * chunkSizePadding) < currentPlaneLimit) {

					// Get the row above the current one
					int planesIndex = row + (width * chunkSizePadding);

					// Get the bits spanning height for the next row
					uint64_t nextRowHeight = planes[planesIndex] & heightMask;

					//printBinary(planes[planesIndex], 8, "Plane limit: " + std::to_string(currentPlaneLimit) + " Value to be lower than plane limit: " + std::to_string(row + (width * chunkSizePadding)) + " Next row index: " + std::to_string(row + (width * chunkSizePadding)) + " Width: " + std::to_string(width) + " Next row : ");

					//printBinary(nextRowHeight, 8, "nextRowHeight (next row & heightMask): ");

					//UE_LOG(LogTemp, Warning, TEXT("Is nextRowHeight and heightMask equal: %s"), (nextRowHeight == heightMask) ? TEXT("true") : TEXT("false"));

					if (nextRowHeight != heightMask) {
						break; // Can't expand horizontally
					}

					// Remove the bits we expanded into
					planes[planesIndex] = planes[planesIndex] & ~heightMask;

					//printBinary(planes[planesIndex], 8, "Next row after flipping bits we expanded into: ");

					width++;
				}
				
				break;
		
				

				// break;
			}

			// Create quad from 
					// FVector voxelPosition1 = getVoxelStartingPosition(y, axis, x, z, bitIndex, row); // old version

			
			// THIS IS FOR THE Y AXIS
			FVector voxelPosition1(3);
			FVector voxelPosition2(3);
			FVector voxelPosition3(3);
			FVector voxelPosition4(3);
			double voxelX{ 0.0 };
			double voxelZ{ 0.0 };
			double voxelY{ 0.0 };
			switch (axis) {
			case 0:
			case 1:
				voxelX = static_cast<double>(row % chunkSizePadding);
				voxelZ = static_cast<double>(y);

				voxelY = row > 0 ? static_cast<double>(row / chunkSizePadding) : 0.0;
				voxelPosition1 = { voxelX, voxelZ, voxelY }; // X, Y, Z (height)

				// UE_LOG(LogTemp, Log, TEXT("voxelX: %f, voxelZ: %f, voxelY: %f, width: %d, height: %d, axis: %d"), voxelX, voxelZ, voxelY, width, height, axis);
				
				// Modify the original voxel position and create the remaining three quad position
				createAllVoxelPositionsFromOriginal(voxelPosition1, voxelPosition2, voxelPosition3, voxelPosition4, width, height, axis);

				// Create the quads
				createQuadAndAddToMeshData(voxelPosition1, voxelPosition2, voxelPosition3, voxelPosition4, width, height, axis);
				break;
			case 2:
			case 3:
				voxelZ = static_cast<double>(row % chunkSizePadding);
				voxelX = static_cast<double>(y);

				// Height increases with each 64 rows for X and Z
				voxelY = row > 0 ? std::floor(static_cast<double>(row / chunkSizePadding)) : 0.0;

				voxelPosition1 = { voxelX, voxelZ, voxelY }; // X, Y, Z (height)

				// Modify the original voxel position and create the remaining three quad position
				createAllVoxelPositionsFromOriginal(voxelPosition1, voxelPosition2, voxelPosition3, voxelPosition4, height, width, axis);

				// UE_LOG(LogTemp, Warning, TEXT("Width: %d, Height: %d\n\tVoxel position 1: voxelX: %f, voxelZ: %f, voxelY: %f\n\tVoxel position 2: voxelX: %f, voxelZ: %f, voxelY: %f\n\tVoxel position 3: voxelX: %f, voxelZ: %f, voxelY: %f\n\tVoxel position 4: voxelX: %f, voxelZ: %f, voxelY: %f"), width, height, voxelX, voxelZ, voxelY, voxelPosition2.X, voxelPosition2.Y, voxelPosition2.Z, voxelPosition3.X, voxelPosition3.Y, voxelPosition3.Z, voxelPosition4.X, voxelPosition4.Y, voxelPosition4.Z);

				// Create the quads
				createQuadAndAddToMeshData(voxelPosition1, voxelPosition2, voxelPosition3, voxelPosition4, height, width, axis);
				break;
			case 4:
			case 5:
				voxelX = static_cast<double>(row % chunkSizePadding);
				voxelZ = static_cast<double>(y);

				// Height increases with each 64 rows for X and Z
				voxelY = row > 0 ? std::floor(static_cast<double>(row / chunkSizePadding)) : 0.0;

				voxelPosition1 = { voxelX, voxelZ, voxelY }; // X, Y, Z (height)

				// Modify the original voxel position and create the remaining three quad position
				createAllVoxelPositionsFromOriginal(voxelPosition1, voxelPosition2, voxelPosition3, voxelPosition4, height, width, axis);

				// UE_LOG(LogTemp, Warning, TEXT("Width: %d, Height: %d\n\tVoxel position 1: voxelX: %f, voxelZ: %f, voxelY: %f\n\tVoxel position 2: voxelX: %f, voxelZ: %f, voxelY: %f\n\tVoxel position 3: voxelX: %f, voxelZ: %f, voxelY: %f\n\tVoxel position 4: voxelX: %f, voxelZ: %f, voxelY: %f"), width, height, voxelX, voxelZ, voxelY, voxelPosition2.X, voxelPosition2.Y, voxelPosition2.Z, voxelPosition3.X, voxelPosition3.Y, voxelPosition3.Z, voxelPosition4.X, voxelPosition4.Y, voxelPosition4.Z);

				// Create the quads
				createQuadAndAddToMeshData(voxelPosition1, voxelPosition2, voxelPosition3, voxelPosition4, height, width, axis);
				break;
			}

			

			

		}



	}
}


void ABinaryChunk::testingMeshingCreation() {
	int width{ 1 }; // TODO change after greeding meshing
	int height{ 1 }; // TODO change after greeding meshing


	FVector voxelPosition1 = { 0, 0, 0 };

	for (int axis = 0; axis < 4; axis++) {

		FVector widthPositionModifier = { 0, 0, 0 };
		FVector heightPositionModifier = { 0, 0, 0 };
		FVector voxelPosition2(3);
		FVector voxelPosition3(3);
		FVector voxelPosition4(3);

		// Vector for face directions // TODO Maybe delete this. Double-check after fixing X and Z faces
		FVector bottomFace = { 0, 0, 1 };
		FVector topFace = { 0, 0, -1 };
		FVector leftFace = { 0, 1, 0 };
		FVector rightFace = { 0, -1, 0 };
		FVector forwardFace = { 1, 0, 0 };
		FVector backwardFace = { -1, 0, 0 };

		switch (axis) {
		case 0: // Y axis ascending
			widthPositionModifier[0] = width; // NOT SURE IF IT'S WIDTH HERE
			heightPositionModifier[1] = height; // NOT SURE IF IT'S HEIGHT HERE

			// Adjust face direction
			voxelPosition1 += bottomFace; // TODO Maybe delete this. Double-check after fixing X and Z faces

			voxelPosition2 = voxelPosition1 + heightPositionModifier; // bottom - right
			voxelPosition3 = voxelPosition1 + widthPositionModifier + heightPositionModifier; // top - right
			voxelPosition4 = voxelPosition1 + widthPositionModifier;// top - left
			break;
		case 1: // Y axis descending
			widthPositionModifier[0] = width; // NOT SURE IF IT'S WIDTH HERE
			heightPositionModifier[1] = height; // NOT SURE IF IT'S HEIGHT HERE

			// Adjust face direction
			voxelPosition1 += topFace; // TODO Maybe delete this. Double-check after fixing X and Z faces

			voxelPosition2 = voxelPosition1 + widthPositionModifier; // top - left
			voxelPosition3 = voxelPosition1 + widthPositionModifier + heightPositionModifier; // top - right
			voxelPosition4 = voxelPosition1 + heightPositionModifier; // bottom - right
			break;
		case 2: // X axis descending
			widthPositionModifier[1] = width; // NOT SURE IF IT'S WIDTH HERE
			heightPositionModifier[2] = height; // NOT SURE IF IT'S HEIGHT HERE

			// Adjust face direction
			voxelPosition1 += forwardFace; // TODO Maybe delete this. Double-check after fixing X and Z faces

			voxelPosition2 = voxelPosition1 + heightPositionModifier; // bottom - right
			voxelPosition3 = voxelPosition1 + widthPositionModifier + heightPositionModifier; // top - right
			voxelPosition4 = voxelPosition1 + widthPositionModifier;// top - left
			break;
		case 3: // X axis ascending
			widthPositionModifier[1] = width; // NOT SURE IF IT'S WIDTH HERE
			heightPositionModifier[2] = height; // NOT SURE IF IT'S HEIGHT HERE

			// Adjust face direction
			voxelPosition1 += backwardFace; // TODO Maybe delete this. Double-check after fixing X and Z faces

			voxelPosition2 = voxelPosition1 + widthPositionModifier; // top - left
			voxelPosition3 = voxelPosition1 + widthPositionModifier + heightPositionModifier; // top - right
			voxelPosition4 = voxelPosition1 + heightPositionModifier; // bottom - right
			break;
		case 4: // Z axis descending
			widthPositionModifier[0] = width; // NOT SURE IF IT'S WIDTH HERE
			heightPositionModifier[2] = height; // NOT SURE IF IT'S HEIGHT HERE

			// Adjust face direction
			voxelPosition1 += leftFace; // TODO Maybe delete this. Double-check after fixing X and Z faces

			voxelPosition2 = voxelPosition1 + widthPositionModifier; // bottom - right
			voxelPosition3 = voxelPosition1 + widthPositionModifier + heightPositionModifier; // top - right
			voxelPosition4 = voxelPosition1 + heightPositionModifier;// top - left
			break;
		case 5:	// Z axis ascending
			widthPositionModifier[0] = width; // NOT SURE IF IT'S WIDTH HERE
			heightPositionModifier[2] = height; // NOT SURE IF IT'S HEIGHT HERE
		
			// Adjust face direction
			voxelPosition1 += rightFace; // TODO Maybe delete this. Double-check after fixing X and Z faces

			voxelPosition2 = voxelPosition1 + heightPositionModifier; // top - left
			voxelPosition3 = voxelPosition1 + widthPositionModifier + heightPositionModifier; // top - right
			voxelPosition4 = voxelPosition1 + widthPositionModifier; // bottom - right
			break;
		default:
			UE_LOG(LogTemp, Error, TEXT("Invalid axis value: %d"), axis);
			ensureMsgf(false, TEXT("Unhandled case in switch statement for axis: %d"), axis);
			break;
		}

		
		// Log voxel positions
		UE_LOG(LogTemp, Warning, TEXT("Axis: %d"), axis);
		UE_LOG(LogTemp, Warning, TEXT("VoxelPosition1: %s"), *voxelPosition1.ToString());
		UE_LOG(LogTemp, Warning, TEXT("VoxelPosition2: %s"), *voxelPosition2.ToString());
		UE_LOG(LogTemp, Warning, TEXT("VoxelPosition3: %s"), *voxelPosition3.ToString());
		UE_LOG(LogTemp, Warning, TEXT("VoxelPosition4: %s"), *voxelPosition4.ToString());

		createQuadAndAddToMeshData(voxelPosition1, voxelPosition2, voxelPosition3, voxelPosition4, width, height, axis);
	}
}

void ABinaryChunk::createQuadAndAddToMeshData(
	const FVector& voxelPosition1,
	const FVector& voxelPosition2,
	const FVector& voxelPosition3,
	const FVector& voxelPosition4,
	const int& height, const int& width,
	const int& axis
	) {

	MeshData.Vertices.Append({
		voxelPosition1 * 100,
		voxelPosition2 * 100,
		voxelPosition3 * 100,
		voxelPosition4 * 100
	});

	// Add triangles and increment vertex count
	MeshData.Triangles.Append({
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

	MeshData.Normals.Append({ Normal, Normal, Normal, Normal });

	MeshData.UV0.Append({
		FVector2D(0, 0), FVector2D(0, height), FVector2D(width, height), FVector2D(width, 0)
	});

	// Randomize the RGB values
	uint8 Red = FMath::RandRange(0, 255);
	uint8 Green = FMath::RandRange(0, 255);
	uint8 Blue = FMath::RandRange(0, 255);

	FColor Color = FColor(Red, Green, Blue, 255);

	// Add colors
	MeshData.Colors.Append({
		Color, Color, Color, Color
	});
}

void ABinaryChunk::spawnTerrainChunkMeshes() {
	Mesh->CreateMeshSection(0, MeshData.Vertices, MeshData.Triangles, MeshData.Normals, MeshData.UV0, MeshData.Colors, TArray<FProcMeshTangent>(), false);
}

void ABinaryChunk::printExecutionTime(Time& start, Time& end, const char* functionName) {
	std::chrono::duration<double, std::milli> duration = end - start;
	UE_LOG(LogTemp, Warning, TEXT("%s() took %f milliseconds to execute."), *FString(functionName), duration.count());
}

// Called when the game starts or when spawned
void ABinaryChunk::BeginPlay() {
	Super::BeginPlay();

	Time start = std::chrono::high_resolution_clock::now();

	createBinarySolidColumnsYXZ();

	Time end = std::chrono::high_resolution_clock::now();

	// printExecutionTime(start, end, "createBinarySolidColumnsYXZ");

	start = std::chrono::high_resolution_clock::now();

	createTerrainMeshesData();

	end = std::chrono::high_resolution_clock::now();

	// printExecutionTime(start, end, "createTerrainMeshesData");

	start = std::chrono::high_resolution_clock::now();

	spawnTerrainChunkMeshes();

	end = std::chrono::high_resolution_clock::now();

	// printExecutionTime(start, end, "spawnTerrainChunkMeshes");


	// Testing generating simple cube
	// testingMeshingCreation();
	// generateChunkMeshes();

}

