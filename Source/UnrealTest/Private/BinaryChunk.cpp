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

void ABinaryChunk::populateChunkVertices() {
	//BinaryChunk3D binaryChunk = BinaryChunk3D{};

	const std::array<float, 3> octaveFrequencies{ 0.02f, 0.025f, 0.03f };

	int loops{ 0 }; // TESTING 
	// Loop over the chunk dimensions (X, Y, Z)
	for (int x = 0; x < chunkSize; x++) {
		for (int z = 0; z < chunkSize * chunkSize; z++) {

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
			for (int bitIndex = 0; bitIndex < chunkHeight / chunkSize; bitIndex++) {
				uint64_t yHeight;

				if (height >= 64) {
					yHeight = ~0ULL; // Set all bits to 1
					height -= 64;
				} else {
					yHeight = (1ULL << height) - 1; // Set bits = height
					height = 0;
				}

				// Get index of y 
				int yIndex{ x * chunkSize + (z * chunkSize * chunkSize) + bitIndex + z }; // DOUBLE CHECK THIS

				// Resize vector if not large enough
				if (yIndex >= binaryChunk.voxelsPosition.size()) {
					binaryChunk.voxelsPosition.resize(yIndex + 1);
				}

				// Add blocks height data (Y) to the current X and Z
				binaryChunk.voxelsPosition[yIndex] = yHeight;
				loops++;
			}


		}
	}

	// Testing
	// std::cout << "Loops: " << loops << std::endl;
	// std::cout << "Vector size: " << binaryChunk.voxelsPosition.size() << std::endl;
}

// Called when the game starts or when spawned
void ABinaryChunk::BeginPlay() {
	Super::BeginPlay();

	populateChunkVertices();

}

