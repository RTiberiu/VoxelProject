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
					uint64_t nthBitY = (currentYCol >> y) & 1;

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

					int columnIndex{ (x * chunkSize * intsPerHeight) + (z * intsPerHeight) + bitIndex }; // TODO VALIDATE THIS INDEX

					// UE_LOG(LogTemp, Warning, TEXT("BIT INDEX: %d   --  COLUMN INDEX: %d  --  X: %d Z: %d"), bitIndex, columnIndex, x, z);
					// TODO Remove padding once I add it 

					uint64_t column = columnFaceMasks[axis][columnIndex]; 




					// int xIndex{ (y * chunkSize) + (bitIndex * chunkSize * chunkSize) + x };

					if (axis == 3) { //  && x == 50 && (z >= 35 || z <= 40)
						// UE_LOG(LogTemp, Warning, TEXT("x column when Z is: %d, X: %d, bitIndex: %d, columnIndex: %d"), z, x, bitIndex, columnIndex);
						// printBinary(column, 8);
					}

					// UE_LOG(LogTemp, Warning, TEXT("\tGrabbed the column!"));
					// printBinary(column, 8);

					while (column != 0) {
						// Get y's starting position, depending on the current bitIndex
						int startingPosition = bitIndex * chunkSize;

						// Get the trailing zeros for the current column
						int y = std::countr_zero(column);



						// Clear the position 
						column &= column - 1;

						FVector voxelPosition1(3);
						switch (axis) {
						case 0:
						case 1:
							//									  X                      Y                     Z
							voxelPosition1 = { static_cast<float>(x), static_cast<float>(z), static_cast<float>(y + startingPosition) }; // up / down  // TODO VALIDATE THIS VALUE
							break;
						case 2:
						case 3:
							voxelPosition1 = { static_cast<float>(y), static_cast<float>(z), static_cast<float>(x + startingPosition) }; // right / left // TODO VALIDATE THIS VALUE)
							// Z needs to be increments each time up until 64 and then resetted to 0 and again to 64 and so on
							break;
						case 4:
						case 5:
							voxelPosition1 = { static_cast<float>(x), static_cast<float>(y), static_cast<float>(z + startingPosition) }; // forward / backwards // TODO VALIDATE THIS VALUE
							break;
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
						case 1: // Y axis descending
							widthPositionModifier[0] = width; // NOT SURE IF IT'S WIDTH HERE
							heightPositionModifier[1] = height; // NOT SURE IF IT'S HEIGHT HERE
							voxelPosition2 = voxelPosition1 + widthPositionModifier; // top - left
							voxelPosition3 = voxelPosition1 + widthPositionModifier + heightPositionModifier; // top - right
							voxelPosition4 = voxelPosition1 + heightPositionModifier; // bottom - right
							break;
						case 0: // Y axis ascending
							widthPositionModifier[0] = width; // NOT SURE IF IT'S WIDTH HERE
							heightPositionModifier[1] = height; // NOT SURE IF IT'S HEIGHT HERE
							voxelPosition2 = voxelPosition1 + heightPositionModifier; // bottom - right
							voxelPosition3 = voxelPosition1 + widthPositionModifier + heightPositionModifier; // top - right
							voxelPosition4 = voxelPosition1 + widthPositionModifier;// top - left
							break;
						case 2: // X axis descending
							widthPositionModifier[0] = width; // NOT SURE IF IT'S WIDTH HERE
							heightPositionModifier[2] = height; // NOT SURE IF IT'S HEIGHT HERE
							voxelPosition2 = voxelPosition1 + widthPositionModifier; // bottom - right
							voxelPosition3 = voxelPosition1 + widthPositionModifier + heightPositionModifier; // top - right
							voxelPosition4 = voxelPosition1 + heightPositionModifier;// top - left
							break;
						case 3:	// X axis ascending
							widthPositionModifier[0] = width; // NOT SURE IF IT'S WIDTH HERE
							heightPositionModifier[2] = height; // NOT SURE IF IT'S HEIGHT HERE
							voxelPosition2 = voxelPosition1 + heightPositionModifier; // top - left
							voxelPosition3 = voxelPosition1 + widthPositionModifier + heightPositionModifier; // top - right
							voxelPosition4 = voxelPosition1 + widthPositionModifier; // bottom - right
							break;
						case 4: // Z axis descending
							widthPositionModifier[1] = width; // NOT SURE IF IT'S WIDTH HERE
							heightPositionModifier[2] = height; // NOT SURE IF IT'S HEIGHT HERE
							voxelPosition2 = voxelPosition1 + heightPositionModifier; // bottom - right
							voxelPosition3 = voxelPosition1 + widthPositionModifier + heightPositionModifier; // top - right
							voxelPosition4 = voxelPosition1 + widthPositionModifier;// top - left
							break;
						case 5: // Z axis ascending
							widthPositionModifier[1] = width; // NOT SURE IF IT'S WIDTH HERE
							heightPositionModifier[2] = height; // NOT SURE IF IT'S HEIGHT HERE
							voxelPosition2 = voxelPosition1 + widthPositionModifier; // top - left
							voxelPosition3 = voxelPosition1 + widthPositionModifier + heightPositionModifier; // top - right
							voxelPosition4 = voxelPosition1 + heightPositionModifier; // bottom - right
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
						createQuadAndAddToMeshData(&voxelPosition1, &voxelPosition2, &voxelPosition3, &voxelPosition4, &width, &height);

					}
				}
			}
		}
	}
}

void ABinaryChunk::createQuadAndAddToMeshData(
		FVector* voxelPosition1,
		FVector* voxelPosition2,
		FVector* voxelPosition3,
		FVector* voxelPosition4,
		int* height, int* width
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

	FVector Normal = FVector::CrossProduct(*voxelPosition2 - *voxelPosition1, *voxelPosition4 - *voxelPosition1).GetSafeNormal();
	MeshData.Normals.Append({ Normal, Normal, Normal, Normal });

	MeshData.UV0.Append({
		FVector2D(*width, *height), FVector2D(0, *height), FVector2D(*width, 0), FVector2D(0, 0)
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

void ABinaryChunk::testingMeshCreation() {
	TArray<FVector> Vertices;
	TArray<int32> Triangles;
	TArray<FVector> Normals;
	TArray<FColor> Colors;
	TArray<FVector2D> UV0;
	TArray<FProcMeshTangent> Tangents;

	int VertexCount = 0;
	int Width = 1;
	int Height = 2;

	// x, z, y
	int axis{ 0 };
	FVector voxelPosition1 = { 1, 1, 1 }; // X, Y, Z order in FVector
	// FVector voxelPosition2 = { 0, 1, 1 };

	// for up and down (axis 0 and 1) Y 
		// This points downwards 
		FVector v1_downwards = FVector(voxelPosition1.X, voxelPosition1.Y, voxelPosition1.Z); // bottom-left
		FVector v2_downwards = FVector(voxelPosition1.X + 1, voxelPosition1.Y, voxelPosition1.Z); // top - left
		FVector v3_downwards = FVector(voxelPosition1.X + 1, voxelPosition1.Y + 1, voxelPosition1.Z); // top - right
		FVector v4_downwards = FVector(voxelPosition1.X, voxelPosition1.Y + 1, voxelPosition1.Z); // bottom - right

		// This points upwards
		FVector v1_upwards = FVector(voxelPosition1.X, voxelPosition1.Y, voxelPosition1.Z); // bottom-left
		FVector v2_upwards = FVector(voxelPosition1.X, voxelPosition1.Y + 1, voxelPosition1.Z); // bottom - right
		FVector v3_upwards = FVector(voxelPosition1.X + 1, voxelPosition1.Y + 1, voxelPosition1.Z); // top - right
		FVector v4_upwards = FVector(voxelPosition1.X + 1, voxelPosition1.Y, voxelPosition1.Z); // top - left

	// for left and right (axis 2 and 3) X
		// This points left
		FVector v1_left = FVector(voxelPosition1.X, voxelPosition1.Y, voxelPosition1.Z); // bottom-left
		FVector v2_left = FVector(voxelPosition1.X + 1, voxelPosition1.Y, voxelPosition1.Z); // bottom - right
		FVector v3_left = FVector(voxelPosition1.X + 1, voxelPosition1.Y, voxelPosition1.Z + 1); // top - right
		FVector v4_left = FVector(voxelPosition1.X, voxelPosition1.Y, voxelPosition1.Z + 1); // top - left
	 
		// This points right
		FVector v1_right = FVector(voxelPosition1.X, voxelPosition1.Y, voxelPosition1.Z); // bottom-left
		FVector v2_right = FVector(voxelPosition1.X, voxelPosition1.Y, voxelPosition1.Z + 1); // top - left
		FVector v3_right = FVector(voxelPosition1.X + 1, voxelPosition1.Y, voxelPosition1.Z + 1); // top - right
		FVector v4_right = FVector(voxelPosition1.X + 1, voxelPosition1.Y, voxelPosition1.Z); // bottom - right

	// for forwards and backwards (axis 4 and 5) Z
		// This points forwards 
		FVector v1_forwards = FVector(voxelPosition1.X, voxelPosition1.Y, voxelPosition1.Z); // bottom-left
		FVector v2_forwards = FVector(voxelPosition1.X, voxelPosition1.Y, voxelPosition1.Z + 1); // bottom - right
		FVector v3_forwards = FVector(voxelPosition1.X, voxelPosition1.Y + 1, voxelPosition1.Z + 1); // top - right
		FVector v4_forwards = FVector(voxelPosition1.X, voxelPosition1.Y + 1, voxelPosition1.Z); // top - left

		// This points backwards 
		FVector v1_backwards = FVector(voxelPosition1.X, voxelPosition1.Y, voxelPosition1.Z); // bottom-left
		FVector v2_backwards = FVector(voxelPosition1.X, voxelPosition1.Y + 1, voxelPosition1.Z); // top - left
		FVector v3_backwards = FVector(voxelPosition1.X, voxelPosition1.Y + 1, voxelPosition1.Z + 1); // top - right
		FVector v4_backwards = FVector(voxelPosition1.X, voxelPosition1.Y, voxelPosition1.Z + 1); // bottom - right


	// Define vertices
	/*FVector V1 = FVector(0 * 100, 0 * 100, 0 * 100);
	FVector V2 = FVector(0 * 100, 1 * 100, 0 * 100);
	FVector V3 = FVector(1 * 100, 1 * 100, 0 * 100);
	FVector V4 = FVector(1 * 100, 0 * 100, 0 * 100);*/

	Vertices.Append({
		v1_downwards * 100, v2_downwards * 100, v3_downwards * 100, v4_downwards * 100,
		v1_upwards * 100, v2_upwards * 100, v3_upwards * 100, v4_upwards * 100,
		v1_left * 100, v2_left * 100, v3_left * 100, v4_left * 100,
		v1_right * 100, v2_right * 100, v3_right * 100, v4_right * 100,
		v1_forwards * 100, v2_forwards * 100, v3_forwards * 100, v4_forwards * 100,
		v1_backwards * 100, v2_backwards * 100, v3_backwards * 100, v4_backwards * 100
	});

	// Define triangles
	Triangles.Append({
		VertexCount, VertexCount + 1, VertexCount + 2,
		VertexCount + 2, VertexCount + 3, VertexCount
	});

	VertexCount += 4;

	Triangles.Append({
		VertexCount, VertexCount + 1, VertexCount + 2,
		VertexCount + 2, VertexCount + 3, VertexCount
	});

	VertexCount += 4;

	Triangles.Append({
		VertexCount, VertexCount + 1, VertexCount + 2,
		VertexCount + 2, VertexCount + 3, VertexCount
	});

	VertexCount += 4;

	Triangles.Append({
		VertexCount, VertexCount + 1, VertexCount + 2,
		VertexCount + 2, VertexCount + 3, VertexCount
	});

	VertexCount += 4;

	Triangles.Append({
		VertexCount, VertexCount + 1, VertexCount + 2,
		VertexCount + 2, VertexCount + 3, VertexCount
	});

	VertexCount += 4;

	Triangles.Append({
		VertexCount, VertexCount + 1, VertexCount + 2,
		VertexCount + 2, VertexCount + 3, VertexCount
	});


	// Calculate normals (optional)
	// FVector Normal = FVector::CrossProduct(v2 - v1, v4 - v1).GetSafeNormal(); // Example of calculating normal
	// Normals.Init(Normal, Vertices.Num());

	// Normals for the downward face
	FVector edge1 = v2_downwards - v1_downwards;
	FVector edge2 = v4_downwards - v1_downwards;
	FVector Normal = FVector::CrossProduct(edge2, edge1).GetSafeNormal();
	Normals.Append({ Normal, Normal, Normal, Normal });

	// Normals for the upward face
	edge1 = v2_upwards - v1_upwards;
	edge2 = v4_upwards - v1_upwards;
	Normal = FVector::CrossProduct(edge1, edge2).GetSafeNormal();
	Normals.Append({ Normal, Normal, Normal, Normal });

	// Normals for the left face
	edge1 = v2_left - v1_left;
	edge2 = v4_left - v1_left;
	Normal = FVector::CrossProduct(edge1, edge2).GetSafeNormal();
	Normals.Append({ Normal, Normal, Normal, Normal });

	// Normals for the right face
	edge1 = v2_right - v1_right;
	edge2 = v4_right - v1_right;
	Normal = FVector::CrossProduct(edge2, edge1).GetSafeNormal();
	Normals.Append({ Normal, Normal, Normal, Normal });

	// Normals for the forward face
	edge1 = v2_forwards - v1_forwards;
	edge2 = v4_forwards - v1_forwards;
	Normal = FVector::CrossProduct(edge1, edge2).GetSafeNormal();
	Normals.Append({ Normal, Normal, Normal, Normal });

	// Normals for the backward face
	edge1 = v2_backwards - v1_backwards;
	edge2 = v4_backwards - v1_backwards;
	Normal = FVector::CrossProduct(edge2, edge1).GetSafeNormal();
	Normals.Append({ Normal, Normal, Normal, Normal });


	// Define UV coordinates
	UV0.Append({
		FVector2D(Width, Height), FVector2D(0, Height), FVector2D(Width, 0), FVector2D(0, 0),
		FVector2D(Width, Height), FVector2D(0, Height), FVector2D(Width, 0), FVector2D(0, 0),
		FVector2D(Width, Height), FVector2D(0, Height), FVector2D(Width, 0), FVector2D(0, 0),
		FVector2D(Width, Height), FVector2D(0, Height), FVector2D(Width, 0), FVector2D(0, 0),
		FVector2D(Width, Height), FVector2D(0, Height), FVector2D(Width, 0), FVector2D(0, 0),
		FVector2D(Width, Height), FVector2D(0, Height), FVector2D(Width, 0), FVector2D(0, 0)
	});

	// Optionally define colors (optional)
	FColor Color = FColor(18, 126, 20, 255); // White color
	Colors.Init(Color, Vertices.Num());

	// Create procedural mesh section
	Mesh->CreateMeshSection(0, Vertices, Triangles, TArray<FVector>(), UV0, TArray<FColor>(), TArray<FProcMeshTangent>(), false);


	// Increment vertex count for potential future use
	VertexCount += 4;
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

	// testingMeshCreation();




}

