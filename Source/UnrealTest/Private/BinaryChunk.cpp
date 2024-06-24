// Fill out your copyright notice in the Description page of Project Settings.

#include "BinaryChunk.h"
#include "ProceduralMeshComponent.h"

#include <iostream>
#include <array>
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

void ABinaryChunk::createBinarySolidColumnsYXZ() {
	const std::array<float, 3> octaveFrequencies{ 0.02f, 0.025f, 0.03f };

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

					uint64_t maskX = static_cast<uint64_t>(1) << z;

					// Next Y index (column) means the same X index (column), but a change in Y bit index
					int xIndex{ (y * chunkSize) + (bitIndex * chunkSize * chunkSize) + x };

					// Flip the current X block if it's not already solid 
					if (!(binaryChunk.xBinaryColumn[xIndex] & maskX)) {
						binaryChunk.xBinaryColumn[xIndex] |= maskX;
					}

					// Next Y index (column) means the next Z index (column), but the same Y bit index
					uint64_t maskZ = static_cast<uint64_t>(1) << x;

					int zIndex{ (y * chunkSize) + (bitIndex * chunkSize * chunkSize) + z }; // FUCKING VERIFIED

					if (!(binaryChunk.zBinaryColumn[zIndex] & maskZ)) {
						binaryChunk.zBinaryColumn[zIndex] |= maskZ;

					}

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
}

void ABinaryChunk::faceCullingBinaryColumnsYXZ() {
	// Storing the face masks for the Y, X, Z axis
	std::vector<std::vector<uint64_t>> columnFaceMasks{
		std::vector<uint64_t>(chunkSize * chunkSize * intsPerHeight),
		std::vector<uint64_t>(chunkSize * chunkSize * intsPerHeight),
		std::vector<uint64_t>(chunkSize * chunkSize * intsPerHeight),
	};

	// Face culling for all the 3 axis (Y, X, Z)
	for (int axis = 0; axis < columnFaceMasks.size(); axis++) {

		for (int i = 0; i < binaryChunk.yBinaryColumn.size(); i++) {
			uint64_t column = binaryChunk.yBinaryColumn[i];

			// Sample ascending axis and set to true when air meets solid
			columnFaceMasks[axis][(axis + 1) * chunkSize + i] = column & ~(column >> 1);

			// Sample descending axis and set to true when air meets solid
			columnFaceMasks[axis][axis * chunkSize + i] = column & ~(column << 1);
		}
	}

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
}

