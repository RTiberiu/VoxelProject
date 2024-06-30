// Fill out your copyright notice in the Description page of Project Settings.

#include "BinaryChunk.h"
#include "ProceduralMeshComponent.h"

#include <iostream>
#include <array>
#include <bitset>
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


// Debugging function that prints a 64-bit integer in groups
void ABinaryChunk::printBinary(uint64_t value, int groupSize) {
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
	UE_LOG(LogTemp, Warning, TEXT("Column value: %s"), *formattedFString);

}

void ABinaryChunk::createBinarySolidColumnsYXZ() {
	constexpr std::array<float, 3> octaveFrequencies{ 0.02f, 0.025f, 0.03f };

	// Set the chunk values to air for all 3 axis (Y, X, Z)
	binaryChunk.yBinaryColumn = std::vector<uint64_t>(chunkSize * chunkSize * intsPerHeight, 0);
	binaryChunk.xBinaryColumn = std::vector<uint64_t>(chunkSize * chunkSize * intsPerHeight, 0);
	binaryChunk.zBinaryColumn = std::vector<uint64_t>(chunkSize * chunkSize * intsPerHeight, 0);

	int loops{ 0 }; // TESTING 
	// Loop over the chunk dimensions (X, Y, Z)
	for (int x = 0; x < chunkSize; x++) {
		for (int z = 0; z < chunkSize; z++) {

			// Looping over the different octaves to get the final height
			int amplitude{ 0 };
			int height{ 0 };
			for (const float octave : octaveFrequencies) {
				// Set frequency and get value between 0 and 2
				noise->SetFrequency(octave);
				const float noiseValue = noise->GetNoise(static_cast<float>(x), static_cast<float>(z)) + 1;

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
				int yIndex{ (x * chunkSize * intsPerHeight) + (z * intsPerHeight) + bitIndex };

				// Add blocks height data (Y) to the current X and Z
				binaryChunk.yBinaryColumn[yIndex] = yHeight;
				uint64_t currentYCol = binaryChunk.yBinaryColumn[yIndex]; // TODO MAYBE COMBINE THIS TWO LINES (the one above)

				// Skip iteration if Y column is all air
				if (currentYCol == 0) {
					continue;
				}

				for (int y = 0; y < chunkHeight; y++) {
					// Next Y index (column) means the same X index (column), but a change in Y bit index
					int xIndex{ (y * chunkSize) + (bitIndex * chunkSize * chunkSize) + x };

					// y'th bit of column Y
					uint8_t nthBitY = (currentYCol >> y) & 1;

					// Create temporary variable for column X
					uint64_t xBitTemp = binaryChunk.xBinaryColumn[xIndex];

					// Apply the change to the temporary X bit
					xBitTemp = (xBitTemp >> z) | nthBitY;

					// Assign to actual bit the change
					binaryChunk.xBinaryColumn[xIndex] = (xBitTemp << z) | binaryChunk.xBinaryColumn[xIndex];

					// Next Y index (column) means the next Z index (column), but the same Y bit index
					int zIndex{ (y * chunkSize) + (bitIndex * chunkSize * chunkSize) + z }; // FUCKING VERIFIED

					// Create temporary variable for column Z
					uint64_t zBitTemp = binaryChunk.zBinaryColumn[zIndex];

					// Apply the change to the temporary Z bit
					zBitTemp = (zBitTemp >> x) | nthBitY;

					// Assign to actual bit the change
					binaryChunk.zBinaryColumn[zIndex] = (zBitTemp << z) | binaryChunk.xBinaryColumn[zIndex];

					loops++;
				}

			}
		}
	}

	// Testing
	UE_LOG(LogTemp, Log, TEXT("Loops: %d"), loops);
	UE_LOG(LogTemp, Log, TEXT("Vector size Y: %d"), binaryChunk.yBinaryColumn.size());
	UE_LOG(LogTemp, Log, TEXT("Vector size X: %d"), binaryChunk.xBinaryColumn.size());
	UE_LOG(LogTemp, Log, TEXT("Vector size Z: %d"), binaryChunk.zBinaryColumn.size());

	// TODO FIX xBinaryColumn
	/*for (int i = 0; i < binaryChunk.xBinaryColumn.size(); ++i) {
		uint64_t column = binaryChunk.xBinaryColumn[i];
		UE_LOG(LogTemp, Warning, TEXT("createBinarySolidColumnsYXZ() X Index: %d"), i);
		printBinary(column, 8);
	}*/

	// TODO FIX xBinaryColumn
	/*for (int i = 0; i < binaryChunk.zBinaryColumn.size(); ++i) {
		uint64_t column = binaryChunk.zBinaryColumn[i];
		UE_LOG(LogTemp, Warning, TEXT("createBinarySolidColumnsYXZ() Z Index: %d"), i);
		printBinary(column, 8);
	}*/
	
}


void ABinaryChunk::faceCullingBinaryColumnsYXZ() {
	// Storing the face masks for the Y, X, Z axis
	// Size is doubled to contains both ascending and descending columns 
	std::vector<std::vector<uint64_t>> columnFaceMasks{
		std::vector<uint64_t>(chunkSize * chunkSize * intsPerHeight), // Y ascending
		std::vector<uint64_t>(chunkSize * chunkSize * intsPerHeight), // Y descending
		std::vector<uint64_t>(chunkSize * chunkSize * intsPerHeight), // X ascending
		std::vector<uint64_t>(chunkSize * chunkSize * intsPerHeight), // X descending
		std::vector<uint64_t>(chunkSize * chunkSize * intsPerHeight), // Z ascending
		std::vector<uint64_t>(chunkSize * chunkSize * intsPerHeight), // Z descending
	};

	int logsToPrint{ 30 };

	// Face culling for all the 3 axis (Y, X, Z)
	for (int axis = 0; axis < 3; axis++) {
		for (int x = 0; x < chunkSize; x++) {
			for (int z = 0; z < chunkSize; z++) {


				for (int bitIndex = 0; bitIndex < intsPerHeight; bitIndex++) {
					int columnIndex{ (x * chunkSize * intsPerHeight) + (z * intsPerHeight) + bitIndex };

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

					// TODO Important note:
					// For the Y axis, I should only do face culling IF the bit is not all 1 or when is the last bitIndex
					// That's because I don't want voxels between my height chunks, since I have 4 64-bit for the height. 
					
					// UE_LOG(LogTemp, Warning, TEXT("\tColumn index: %d\n\tIndex ascending: axis: %d  index: %d\n\tIndex descending: axis: %d  index: %d"), columnIndex, axis * 2 + 0, columnIndex + chunkSize, axis * 2 + 1, columnIndex);
					
					// Sample ascending axis and set to true when air meets solid
					columnFaceMasks[axis * 2 + 0][columnIndex] = column & ~(column >> 1); // TODO VALIDATE THIS INDEX

					// Sample descending axis and set to true when air meets solid
					columnFaceMasks[axis * 2 + 1][columnIndex] = column & ~(column << 1); // TODO VALIDATE THIS INDEX
					
				}

			}
		}
	}

	// Find faces and build binary planes based on the voxel block
	for (int axis = 0; axis < 6; axis++) { // Iterate all axis ascending and descending // 6 value

		// TESTING
		/*if (axis == 1) {
			logsToPrint = 30;
			UE_LOG(LogTemp, Warning, TEXT("Moved to axis 1"));
		} else if (axis == 2) {
			logsToPrint = 30;
			UE_LOG(LogTemp, Warning, TEXT("Moved to axis 2"));
		} else if (axis == 3) {
			logsToPrint = 30;
			UE_LOG(LogTemp, Warning, TEXT("Moved to axis 3"));
		} else if (axis == 4) {
			logsToPrint = 30;
			UE_LOG(LogTemp, Warning, TEXT("Moved to axis 4"));
		} else if (axis == 5) {
			logsToPrint = 30;
			UE_LOG(LogTemp, Warning, TEXT("Moved to axis 5"));
		}*/

		for (int x = 0; x < chunkSize; x++) {
			for (int z = 0; z < chunkSize; z++) {

				for (int bitIndex = 0; bitIndex < intsPerHeight; bitIndex++) {

					int columnIndex{ (x * chunkSize * intsPerHeight) + (z * intsPerHeight) + bitIndex }; // VERIFIED! Goes from 0 - 16,383

					// UE_LOG(LogTemp, Warning, TEXT("BIT INDEX: %d   --  COLUMN INDEX: %d  --  X: %d Z: %d"), bitIndex, columnIndex, x, z);
					// TODO Remove padding once I add it 

					uint64_t column = columnFaceMasks[axis][columnIndex]; 

					if (axis == 3) { //  && x == 50 && (z >= 35 || z <= 40)
						// UE_LOG(LogTemp, Warning, TEXT("x column when Z is: %d, X: %d, bitIndex: %d
						// , columnIndex: %d"), z, x, bitIndex, columnIndex);
						// printBinary(column, 8);
					}

					// UE_LOG(LogTemp, Warning, TEXT("\tGrabbed the column!"));
					// printBinary(column, 8);

					while (column != 0) {

						// TODO
						// The problem seems to be either with the faces, or how the faces get built.
						// I think the X and Z positions are correct now, since I am waiting for a
						// 64x64 chunk to be filled before I increment the height to 64.
						// 
						// Things to double-check:
						// 1.	How X and Z gets added to the array (I think this is correct already, as
						//		I fixed it yesterday).
						// 2.	How the faces get built with createQuadAndAddToMeshData()
						//		2.1. Check the vectors passed to this function.
						//		2.2. Check if the faces are pointing at the right direction.
						//		2.3. Check if the faces are spawning at the right location.
						//		Note: Do this steps 2. in a small test environment (one cube only).

						// Get the trailing zeros for the current column
						int y = std::countr_zero(column);


						// Clear the position 
						column &= column - 1;

						FVector voxelPosition1(3);
						switch (axis) {
						case 0:
						case 1: {
							// Get y's starting position, depending on the current bitIndex
							int startingPositionY = bitIndex * chunkSize;
							//									  X                      Y                     Z
							voxelPosition1 = { static_cast<float>(x), static_cast<float>(z), static_cast<float>(y + startingPositionY) }; // up / down  // TODO VALIDATE THIS VALUE
							break;
						}
						case 2:
						case 3: {
							int startingPositionX = columnIndex > 0 ? (columnIndex / (chunkSize * chunkSize)) * chunkSize : 0;
							voxelPosition1 = { static_cast<float>(y), static_cast<float>(z), static_cast<float>(x + startingPositionX) }; // right / left // TODO VALIDATE THIS VALUE)
							// Z needs to be increments each time up until 64 and then resetted to 0 and again to 64 and so on
							break;
						}
						case 4:
						case 5: {
							int startingPositionZ = columnIndex > 0 ? (columnIndex / (chunkSize * chunkSize)) * chunkSize : 0;
							voxelPosition1 = { static_cast<float>(x), static_cast<float>(y), static_cast<float>(z + startingPositionZ) }; // forward / backwards // TODO VALIDATE THIS VALUE
							break;
						}
						default:
							UE_LOG(LogTemp, Error, TEXT("Invalid axis value: %d"), axis);
							ensureMsgf(false, TEXT("Unhandled case in switch statement for axis: %d"), axis);
							break;
						}


						// DEBUGGING
						if (axis == 2) { // logsToPrint > 0 && 
							// UE_LOG(LogTemp, Warning, TEXT("faceCullingBinaryColumnsYXZ(): \n\taxis: %d\n\tloop values x: %d z: %d columnIndex: %d\n\tvoxelPosition1 x: %f y: %f z: %f\n\tbitIndex: %d"), axis, x, z, columnIndex, voxelPosition1.X, voxelPosition1.Y, voxelPosition1.Z, bitIndex);
							
								
							logsToPrint--;
						}

						// TODO Add greedy meshing and get the height and width 
						

						int width{ 1 }; // TODO change after greeding meshing
						int height{ 1 }; // TODO change after greeding meshing

						
						// Get position modifiers depending on the current axis
						// This values are used to create the 4 quad positions
							// TODO Potentially integrate with the switch above
						FVector widthPositionModifier = {0, 0, 0};
						FVector heightPositionModifier = { 0, 0, 0 };
						FVector voxelPosition2(3);
						FVector voxelPosition3(3);
						FVector voxelPosition4(3);

						switch (axis) {
						case 0: // Y axis ascending
							widthPositionModifier[0] = width; // NOT SURE IF IT'S WIDTH HERE
							heightPositionModifier[1] = height; // NOT SURE IF IT'S HEIGHT HERE
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
						case 2:	// X axis ascending
							widthPositionModifier[0] = width; // NOT SURE IF IT'S WIDTH HERE
							heightPositionModifier[2] = height; // NOT SURE IF IT'S HEIGHT HERE
							voxelPosition2 = voxelPosition1 + heightPositionModifier; // top - left
							voxelPosition3 = voxelPosition1 + widthPositionModifier + heightPositionModifier; // top - right
							voxelPosition4 = voxelPosition1 + widthPositionModifier; // bottom - right
							break;
						case 3: // X axis descending
							widthPositionModifier[0] = width; // NOT SURE IF IT'S WIDTH HERE
							heightPositionModifier[2] = height; // NOT SURE IF IT'S HEIGHT HERE
							voxelPosition2 = voxelPosition1 + widthPositionModifier; // bottom - right
							voxelPosition3 = voxelPosition1 + widthPositionModifier + heightPositionModifier; // top - right
							voxelPosition4 = voxelPosition1 + heightPositionModifier;// top - left
							break;
						case 4: // Z axis ascending
							widthPositionModifier[1] = width; // NOT SURE IF IT'S WIDTH HERE
							heightPositionModifier[2] = height; // NOT SURE IF IT'S HEIGHT HERE
							voxelPosition2 = voxelPosition1 + widthPositionModifier; // top - left
							voxelPosition3 = voxelPosition1 + widthPositionModifier + heightPositionModifier; // top - right
							voxelPosition4 = voxelPosition1 + heightPositionModifier; // bottom - right
							break;
						case 5: // Z axis descending
							widthPositionModifier[1] = width; // NOT SURE IF IT'S WIDTH HERE
							heightPositionModifier[2] = height; // NOT SURE IF IT'S HEIGHT HERE
							voxelPosition2 = voxelPosition1 + heightPositionModifier; // bottom - right
							voxelPosition3 = voxelPosition1 + widthPositionModifier + heightPositionModifier; // top - right
							voxelPosition4 = voxelPosition1 + widthPositionModifier;// top - left
							break;
						default:
							UE_LOG(LogTemp, Error, TEXT("Invalid axis value: %d"), axis);
							ensureMsgf(false, TEXT("Unhandled case in switch statement for axis: %d"), axis);
							break;
						}

						/*UE_LOG(LogTemp, Warning, TEXT("Voxel Positions:\nvoxelPosition1: x: %f y: %f z: %f\nvoxelPosition2: x: %f y: %f z: %f\nvoxelPosition3: x: %f y: %f z: %f\nvoxelPosition4: x: %f y: %f z: %f"),
							voxelPosition1.X, voxelPosition1.Y, voxelPosition1.Z,
							voxelPosition2.X, voxelPosition2.Y, voxelPosition2.Z,
							voxelPosition3.X, voxelPosition3.Y, voxelPosition3.Z,
							voxelPosition4.X, voxelPosition4.Y, voxelPosition4.Z);*/
						// TESTING
						/*if (logsToPrint > 0) {
							logsToPrint--;
						}*/

						// Create the quads
						createQuadAndAddToMeshData(&voxelPosition1, &voxelPosition2, &voxelPosition3, &voxelPosition4, &width, &height, &axis);

					}
				}
				UE_LOG(LogTemp, Warning, TEXT("--------------------- Z: %d"), z);
			}
			UE_LOG(LogTemp, Warning, TEXT("####################### X: %d"), x);
		}
		UE_LOG(LogTemp, Warning, TEXT("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$ AXIS: % d"), axis);
	}
}

void ABinaryChunk::testingMeshingCreation() {
	int width{ 1 }; // TODO change after greeding meshing
	int height{ 1 }; // TODO change after greeding meshing


	FVector voxelPosition1 = { 0, 0, 0 };

	for (int axis = 0; axis < 6; axis++) {

		FVector widthPositionModifier = { 0, 0, 0 };
		FVector heightPositionModifier = { 0, 0, 0 };
		FVector voxelPosition2(3);
		FVector voxelPosition3(3);
		FVector voxelPosition4(3);

		

		switch (axis) {
		case 0: // Y axis ascending

			voxelPosition1 = { 0, 0, 1 }; // bottom face


			widthPositionModifier[0] = width; // NOT SURE IF IT'S WIDTH HERE
			heightPositionModifier[1] = height; // NOT SURE IF IT'S HEIGHT HERE
			voxelPosition2 = voxelPosition1 + heightPositionModifier; // bottom - right
			voxelPosition3 = voxelPosition1 + widthPositionModifier + heightPositionModifier; // top - right
			voxelPosition4 = voxelPosition1 + widthPositionModifier;// top - left
			break;
		case 1: // Y axis descending

			voxelPosition1 = { 0, 0, 0 }; // top face

			widthPositionModifier[0] = width; // NOT SURE IF IT'S WIDTH HERE
			heightPositionModifier[1] = height; // NOT SURE IF IT'S HEIGHT HERE
			voxelPosition2 = voxelPosition1 + widthPositionModifier; // top - left
			voxelPosition3 = voxelPosition1 + widthPositionModifier + heightPositionModifier; // top - right
			voxelPosition4 = voxelPosition1 + heightPositionModifier; // bottom - right
			break;
		case 2:	// X axis ascending

			voxelPosition1 = { 0, 0, 0 }; // right face

			widthPositionModifier[0] = width; // NOT SURE IF IT'S WIDTH HERE
			heightPositionModifier[2] = height; // NOT SURE IF IT'S HEIGHT HERE
			voxelPosition2 = voxelPosition1 + heightPositionModifier; // top - left
			voxelPosition3 = voxelPosition1 + widthPositionModifier + heightPositionModifier; // top - right
			voxelPosition4 = voxelPosition1 + widthPositionModifier; // bottom - right
			break;
		case 3: // X axis descending

			voxelPosition1 = { 0, 1, 0 }; // left face

			widthPositionModifier[0] = width; // NOT SURE IF IT'S WIDTH HERE
			heightPositionModifier[2] = height; // NOT SURE IF IT'S HEIGHT HERE
			voxelPosition2 = voxelPosition1 + widthPositionModifier; // bottom - right
			voxelPosition3 = voxelPosition1 + widthPositionModifier + heightPositionModifier; // top - right
			voxelPosition4 = voxelPosition1 + heightPositionModifier;// top - left
			break;
		case 4: // Z axis ascending

			voxelPosition1 = { 0, 0, 0 }; // backwards face


			widthPositionModifier[1] = width; // NOT SURE IF IT'S WIDTH HERE
			heightPositionModifier[2] = height; // NOT SURE IF IT'S HEIGHT HERE
			voxelPosition2 = voxelPosition1 + widthPositionModifier; // top - left
			voxelPosition3 = voxelPosition1 + widthPositionModifier + heightPositionModifier; // top - right
			voxelPosition4 = voxelPosition1 + heightPositionModifier; // bottom - right
			break;
		case 5: // Z axis descending

			voxelPosition1 = { 1, 0, 0 }; // forwards face


			widthPositionModifier[1] = width; // NOT SURE IF IT'S WIDTH HERE
			heightPositionModifier[2] = height; // NOT SURE IF IT'S HEIGHT HERE
			voxelPosition2 = voxelPosition1 + heightPositionModifier; // bottom - right
			voxelPosition3 = voxelPosition1 + widthPositionModifier + heightPositionModifier; // top - right
			voxelPosition4 = voxelPosition1 + widthPositionModifier;// top - left
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

		createQuadAndAddToMeshData(&voxelPosition1, &voxelPosition2, &voxelPosition3, &voxelPosition4, &width, &height, &axis);
	}
}

void ABinaryChunk::createQuadAndAddToMeshData(
		FVector* voxelPosition1,
		FVector* voxelPosition2,
		FVector* voxelPosition3,
		FVector* voxelPosition4,
		int* height, int* width,
		int* axis
	) {
	// TODO Continue from here 
	// I got to create the vectors from the voxelPosition vector (with the XYZ values)
	//		To this part, I might need to use the axisDirection. 
	//		And maybe calculate it once at the beginning of the axis loop. (so it doesn't repeat with every iteration)
	// Then to create the triangles, that's easy. Just use the vertexCount
	// Then calculate the normal
	// Then build the UV0
	// and finally, add all these calculations and vectors to:
	//			MeshData.Vertices
	//			MeshData.Triangles
	//			MeshData.Normals
	//			MeshData.Colors
	//			MeshData.UV0
	

	MeshData.Vertices.Append({
		*voxelPosition1 * 100,
		*voxelPosition2 * 100,
		*voxelPosition3 * 100,
		*voxelPosition4 * 100
	});

	// Add triangles and increment vertex count
	MeshData.Triangles.Append({
		vertexCount, vertexCount + 1, vertexCount + 2,
		vertexCount + 2, vertexCount + 3, vertexCount
	});

	vertexCount += 4;

	FVector Normal;
	if (*axis == 0 || *axis == 2 || *axis == 4) {
		// Calculate the normals for counter clockwise vectors arrangement
		Normal = FVector::CrossProduct(*voxelPosition4 - *voxelPosition1, *voxelPosition2 - *voxelPosition1).GetSafeNormal();
	} else {
		// Calculate the normals for clockwise vectors arrangement
		Normal = FVector::CrossProduct(*voxelPosition2 - *voxelPosition1, *voxelPosition4 - *voxelPosition1).GetSafeNormal();
	}

	MeshData.Normals.Append({ Normal, Normal, Normal, Normal });

	MeshData.UV0.Append({
		FVector2D(0, 0), FVector2D(0, *height), FVector2D(*width, *height), FVector2D(*width, 0)
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

void ABinaryChunk::generateChunkMeshes() {
	Mesh->CreateMeshSection(0, MeshData.Vertices, MeshData.Triangles, MeshData.Normals, MeshData.UV0, MeshData.Colors, TArray<FProcMeshTangent>(), false);
}

void ABinaryChunk::printExecutionTime(Time& start, Time& end, const char* functionName) {
	std::chrono::duration<double, std::milli> duration = end - start;
	UE_LOG(LogTemp, Log, TEXT("%s() took %f milliseconds to execute."), *FString(functionName), duration.count());
}

// Called when the game starts or when spawned
void ABinaryChunk::BeginPlay() {
	Super::BeginPlay();

	Time start = std::chrono::high_resolution_clock::now();

	 createBinarySolidColumnsYXZ();

	Time end = std::chrono::high_resolution_clock::now();

	printExecutionTime(start, end, "createBinarySolidColumnsYXZ");

	start = std::chrono::high_resolution_clock::now();

	 faceCullingBinaryColumnsYXZ();

	end = std::chrono::high_resolution_clock::now();

	printExecutionTime(start, end, "faceCullingBinaryColumnsYXZ");

	start = std::chrono::high_resolution_clock::now();

	 generateChunkMeshes();

	end = std::chrono::high_resolution_clock::now();

	printExecutionTime(start, end, "generateChunkMeshes");


	// Testing generating simple cube
	// testingMeshingCreation();
	// generateChunkMeshes();

}

