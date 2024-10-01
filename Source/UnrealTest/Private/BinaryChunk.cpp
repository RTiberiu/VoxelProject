// Fill out your copyright notice in the Description page of Project Settings.

#include "BinaryChunk.h"
#include "WorldTerrainSettings.h"
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

	// Declaring the 3 noise objects used to create the terrain
	initializePerlinNoise(continentalness);
	initializePerlinNoise(erosion);
	initializePerlinNoise(peaksAndValleys);

	// Declaring the 3 noise domain warp objects to modify the terrain
	initializeDomainWarpNoise(continentalnessDW);
	initializeDomainWarpNoise(erosionDW);
	initializeDomainWarpNoise(peaksAndValleysDW);

	Mesh->SetCastShadow(false);

	// Set mesh as root
	SetRootComponent(Mesh);
}

void ABinaryChunk::initializePerlinNoise(TObjectPtr<FastNoiseLite>& noise) {
	noise = new FastNoiseLite();
	noise->SetNoiseType(FastNoiseLite::NoiseType_Perlin); 
	noise->SetFractalType(FastNoiseLite::FractalType_FBm);
}

void ABinaryChunk::applyDomainWarpSettings(TObjectPtr<FastNoiseLite>& domainWarp, const int& settingsIndex) {
	// Adding domain warp settings
	domainWarp->SetDomainWarpAmp(PNSR->noiseMapSettings[settingsIndex].DomainWarpAmp);
	domainWarp->SetFrequency(PNSR->noiseMapSettings[settingsIndex].DomainWarpFrequencies);
	domainWarp->SetFractalOctaves(PNSR->noiseMapSettings[settingsIndex].DomainWarpOctaves);
	domainWarp->SetFractalLacunarity(PNSR->noiseMapSettings[settingsIndex].DomainWarpLacunarity);
	domainWarp->SetFractalGain(PNSR->noiseMapSettings[settingsIndex].DomainWarpGain);
}

// Use the settingsIndex to apply the correct noise settings from PerlinNoiseSettings.cpp
void ABinaryChunk::applyPerlinNoiseSettings(TObjectPtr<FastNoiseLite>& noise, const int& settingsIndex) {
	// Set perlin noise settings
	noise->SetFractalOctaves(PNSR->noiseMapSettings[settingsIndex].Octaves);
	noise->SetFrequency(PNSR->noiseMapSettings[settingsIndex].Frequencies);
	noise->SetFractalLacunarity(PNSR->noiseMapSettings[settingsIndex].Lacunarity);
	noise->SetFractalGain(PNSR->noiseMapSettings[settingsIndex].Gain);
	noise->SetFractalWeightedStrength(PNSR->noiseMapSettings[settingsIndex].WeightedStrength);
}

// Use the settingsIndex to apply the correct noise settings from PerlinNoiseSettings.cpp
void ABinaryChunk::initializeDomainWarpNoise(TObjectPtr<FastNoiseLite>& domainWarp) {
	domainWarp = new FastNoiseLite();
	domainWarp->SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
	domainWarp->SetFractalType(FastNoiseLite::FractalType_DomainWarpProgressive);
}

ABinaryChunk::~ABinaryChunk() {
}

void ABinaryChunk::SetWorldTerrainSettings(UWorldTerrainSettings* InWorldTerrainSettings) {
	WorldTerrainSettingsRef = InWorldTerrainSettings;
}

void ABinaryChunk::SetPerlinNoiseSettings(APerlinNoiseSettings* InPerlinNoiseSettings) {
	PerlinNoiseSettingsRef = InPerlinNoiseSettings;
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

void ABinaryChunk::apply3DNoiseToHeightColumn(uint64_t& column, int& x, int& z, int& y, int& bitIndex, const FVector& chunkWorldLocation, int& height) {
	// Return early if total height is bigger than the threshold
	if (height > 50) {
		return;
	}

	// Getting perlin noise position, adjusted to the Unreal Engine grid system 
	const float noisePositionX = static_cast<float>((x * WTSR->UnrealScale + chunkWorldLocation.X) / WTSR->UnrealScale);
	const float noisePositionZ = static_cast<float>((z * WTSR->UnrealScale + chunkWorldLocation.Y) / WTSR->UnrealScale);
	const float noisePositionY = static_cast<float>(((y + bitIndex * WTSR->chunkSizePadding) * WTSR->UnrealScale + chunkWorldLocation.Z) / WTSR->UnrealScale);
	
	const float squashingFactor = (y + bitIndex * WTSR->chunkSizePadding) * PNSR->squashingFactor;

	// noise->SetFrequency(PNSR->noiseFrequency3D);

	// const float noiseValue3D = noise->GetNoise(noisePositionX, noisePositionZ, noisePositionY) + 1;

	// TODO Add 3d noise depending on the height of the actual 2D noise
	if (y + bitIndex * WTSR->chunkSizePadding < 50 && height < 50) { // y + bitIndex * WTSR->chunkSizePadding > 100)
		column |= 1ULL << y;
	}
}

// Axis should be true for X axis, and false for the Z axis (Unreal's Y axis) 
int ABinaryChunk::getRelativePositionInBiomeForAxis(const FVector& chunkWorldLocation, const int& voxelLocation, const bool& axis) {
	double chunkLocation = 0;
	if (axis) { // X axis
		chunkLocation = chunkWorldLocation.X;
	} else { // Y axis getRelativePositionInBiomeForAxis
		chunkLocation = chunkWorldLocation.Y;
	}

	const int relativePositionInBiome = (abs(static_cast<int>(chunkLocation / WTSR->UnrealScale)) + voxelLocation) % WTSR->biomeWidth;

	return relativePositionInBiome;
}

// Axis should be true for X axis, and false for the Z axis (Unreal's Y axis) 
float ABinaryChunk::getVoxelInterpolatedHeightOnAxis(const float& currentBiomeVoxelHeight, const float& adjacentBiomeVoxelHeight, const int& positionInBiome, const bool& axis) {
	float weight = 0.0f;
	float interpolatedHeight = 0.0f;

	// Determine if within blending zone 
	const float blendStartBiomeEnd = WTSR->biomeWidth - WTSR->blendBiomeThreshold;
	const float blendStartBiomeStart = WTSR->blendBiomeThreshold;

	// Calculate weight based on how close the current position is to the end of the biome
	if (positionInBiome > blendStartBiomeEnd) {
		weight = static_cast<float>(WTSR->biomeWidth - positionInBiome) / WTSR->blendBiomeThreshold;
		interpolatedHeight = std::lerp(currentBiomeVoxelHeight, adjacentBiomeVoxelHeight, weight);
	} else if (positionInBiome < blendStartBiomeStart) {
		weight = static_cast<float>(WTSR->blendBiomeThreshold - positionInBiome) / WTSR->blendBiomeThreshold;
		interpolatedHeight = std::lerp(adjacentBiomeVoxelHeight, currentBiomeVoxelHeight, weight);
	}

	return interpolatedHeight;
}

int ABinaryChunk::getColorIndexFromVoxelHeight(const FVector& voxelPosition) {
	const int voxelHeight = static_cast<int>(voxelPosition.Z);
	int colorIndex = 0;

	constexpr int layerHeight = 20;
	constexpr int layers = 9;

	while (colorIndex < layers) {
		const int maxLayerHeight = colorIndex * layerHeight + layerHeight;
		if (voxelHeight < maxLayerHeight) {
			break;
		}
		colorIndex++;
	}

	return colorIndex;
}



void ABinaryChunk::createBinarySolidColumnsYXZ() { // WORK IN PROGRESS! The old createBinarySolidColumnsYXZ() is below and working
	// Get current chunk world position
	const FVector chunkWorldLocation = GetActorLocation();

	// Set the chunk values to air for all 3 axis (Y, X, Z)
	binaryChunk.yBinaryColumn = std::vector<uint64_t>(WTSR->chunkSizePadding * WTSR->chunkSizePadding * WTSR->intsPerHeight, 0);
	binaryChunk.xBinaryColumn = std::vector<uint64_t>(WTSR->chunkSizePadding * WTSR->chunkSizePadding * WTSR->intsPerHeight, 0);
	binaryChunk.zBinaryColumn = std::vector<uint64_t>(WTSR->chunkSizePadding * WTSR->chunkSizePadding * WTSR->intsPerHeight, 0);
#
	// Loop over the chunk dimensions (X, Y, Z)
	for (int x = 0; x < WTSR->chunkSizePadding; x++) {
		for (int z = 0; z < WTSR->chunkSizePadding; z++) {

			// Looping over the different octaves to get the final height
			int amplitude{ 0 };

			// Getting perlin noise position, adjusted to the Unreal Engine grid system 
			float noisePositionX = static_cast<float>((x * WTSR->UnrealScale + chunkWorldLocation.X) / WTSR->UnrealScale);
			float noisePositionZ = static_cast<float>((z * WTSR->UnrealScale + chunkWorldLocation.Y) / WTSR->UnrealScale);

			// Apply domain warp to each noise map and get value
			continentalnessDW->DomainWarp(noisePositionX, noisePositionZ);
			erosionDW->DomainWarp(noisePositionX, noisePositionZ);
			peaksAndValleysDW->DomainWarp(noisePositionX, noisePositionZ);

			const float continentalnessVal = continentalness->GetNoise(noisePositionX, noisePositionZ) + 1;
			const float erosionVal = erosion->GetNoise(noisePositionX, noisePositionZ) + 1;
			const float peaksAndValleysVal = peaksAndValleys->GetNoise(noisePositionX, noisePositionZ) + 1;

			// Adding multiple splines to the continentalness (inland)
			float continentalnessHeight = 0.0f;
			float erosionHeight = 0.0f;

			if (continentalnessVal <= 1.5) {
				float normalizedContNoise = (continentalnessVal - 0.5f) / 1.0f;
				PNSR->noiseMapSettings[0].Amplitudes = std::lerp(30, 150, normalizedContNoise);

				continentalnessHeight = continentalnessVal * PNSR->noiseMapSettings[0].Amplitudes;

				// Adding erosion 
				PNSR->noiseMapSettings[1].Amplitudes = 20;
				float normalizedErosNoise = (erosionVal - 0.5f) / 1.0f;
				erosionHeight = erosionVal * PNSR->noiseMapSettings[1].Amplitudes;

				// Adding peaks and valleys
				if (erosionVal <= 1) {
					PNSR->noiseMapSettings[2].Amplitudes = 5;
				} else if (erosionVal <= 1.5) {
					PNSR->noiseMapSettings[2].Amplitudes = 10;
				} else {
					PNSR->noiseMapSettings[2].Amplitudes = 15;
				}
			}  else {
				continentalnessHeight = 150;

				// Adding erosion 
				PNSR->noiseMapSettings[1].Amplitudes = 85;
				erosionHeight = erosionVal * PNSR->noiseMapSettings[1].Amplitudes;

				// Adding peaks and valleys
				if (erosionVal <= 1) {
					PNSR->noiseMapSettings[2].Amplitudes = 20;
				} else if (erosionVal <= 1.5) {
					PNSR->noiseMapSettings[2].Amplitudes = 25;
				} else {
					PNSR->noiseMapSettings[2].Amplitudes = 30;
				}
			}
			
			// Adding multiple splines to the erosion (height)
			const float peaksAndValleysHeight = peaksAndValleysVal * PNSR->noiseMapSettings[2].Amplitudes;

			const int combinedNoiseHeight = static_cast<int>(std::floor(continentalnessHeight + erosionHeight + peaksAndValleysHeight));

			// Ensuring height remains between chunk borders
			int height = std::clamp(combinedNoiseHeight, 0, static_cast<int>(WTSR->chunkHeight));

			// Add enough bits to y to cover the entire height (4 64bit integers when the max height is 256)
			for (int bitIndex = 0; bitIndex < WTSR->intsPerHeight; bitIndex++) {
				uint64_t yHeight;

				if (height >= 64) {
					yHeight = ~0ULL; // Set all bits to 1
					height -= 64;
				} else {
					yHeight = (1ULL << height) - 1; // Set bits = height
					height = 0;
				}

				// Get index of y 
				const int yIndex{ (x * WTSR->chunkSizePadding * WTSR->intsPerHeight) + (z * WTSR->intsPerHeight) + bitIndex };

				// Add blocks height data (Y) to the current X and Z
				binaryChunk.yBinaryColumn[yIndex] = yHeight;

				// Skip iteration if Y column is all air
				if (binaryChunk.yBinaryColumn[yIndex] == 0) {
					continue;
				}

				for (int y = 0; y < WTSR->chunkSizePadding; y++) {
					// Apply 3D noise to the Y column
					// apply3DNoiseToHeightColumn(binaryChunk.yBinaryColumn[yIndex], x, z, y, bitIndex, chunkWorldLocation, height);

					const uint64_t currentYCol = binaryChunk.yBinaryColumn[yIndex];

					// Next Y index (column) means the same X index (column), but a change in Y bit index
					const int xIndex{ (y * WTSR->chunkSizePadding) + (bitIndex * WTSR->chunkSizePadding * WTSR->chunkSizePadding) + x };

					// y'th bit of column Y
					const uint8_t nthBitY = (currentYCol >> y) & 1;

					// Create temporary variable for column X
					uint64_t xBitTemp = binaryChunk.xBinaryColumn[xIndex];

					// Apply the change to the temporary X bit
					xBitTemp = (xBitTemp >> z) | nthBitY;

					// Assign to actual bit the change
					binaryChunk.xBinaryColumn[xIndex] = (xBitTemp << z) | binaryChunk.xBinaryColumn[xIndex];

					// Next Y index (column) means the next Z index (column), but the same Y bit index
					const int zIndex{ (y * WTSR->chunkSizePadding) + (bitIndex * WTSR->chunkSizePadding * WTSR->chunkSizePadding) + z };

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
		for (int x = 0; x < WTSR->chunkSizePadding; x++) {
			for (int z = 0; z < WTSR->chunkSizePadding; z++) {
				for (int bitIndex = 0; bitIndex < WTSR->intsPerHeight; bitIndex++) {
					const int columnIndex{ (x * WTSR->chunkSizePadding * WTSR->intsPerHeight) + (z * WTSR->intsPerHeight) + bitIndex };

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
					if (axis == 0 && bitIndex < WTSR->intsPerHeight - 1) {
						const bool isAboveSolid = binaryChunk.yBinaryColumn[columnIndex + 1] != 0;
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
							const uint64_t leftmostBitMask = 1ULL << 63;
							const bool isLeftmostBitSet = (binaryChunk.yBinaryColumn[columnIndex - 1] & leftmostBitMask) != 0;
						
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


void ABinaryChunk::createTerrainMeshesData() {
	// Storing the face masks for the Y, X, Z axis
	// Size is doubled to contains both ascending and descending columns 
	std::vector<std::vector<uint64_t>> columnFaceMasks{
		std::vector<uint64_t>(WTSR->chunkSizePadding * WTSR->chunkSizePadding * WTSR->intsPerHeight), // Y ascending
		std::vector<uint64_t>(WTSR->chunkSizePadding * WTSR->chunkSizePadding * WTSR->intsPerHeight), // Y descending
		std::vector<uint64_t>(WTSR->chunkSizePadding * WTSR->chunkSizePadding * WTSR->intsPerHeight), // X ascending
		std::vector<uint64_t>(WTSR->chunkSizePadding * WTSR->chunkSizePadding * WTSR->intsPerHeight), // X descending
		std::vector<uint64_t>(WTSR->chunkSizePadding * WTSR->chunkSizePadding * WTSR->intsPerHeight), // Z ascending
		std::vector<uint64_t>(WTSR->chunkSizePadding * WTSR->chunkSizePadding * WTSR->intsPerHeight), // Z descending
	};

	// Face cull the binary columns on the 3 axis, ascending and descending
	faceCullingBinaryColumnsYXZ(columnFaceMasks);

	// Storing planes for all axis, ascending and descending
	std::vector<std::vector<uint64_t>> binaryPlanes{ 
		std::vector<uint64_t>(WTSR->chunkSizePadding * WTSR->chunkSizePadding * WTSR->intsPerHeight), // Y ascending
		std::vector<uint64_t>(WTSR->chunkSizePadding * WTSR->chunkSizePadding * WTSR->intsPerHeight), // Y descending
		std::vector<uint64_t>(WTSR->chunkSizePadding * WTSR->chunkSizePadding * WTSR->intsPerHeight), // X ascending
		std::vector<uint64_t>(WTSR->chunkSizePadding * WTSR->chunkSizePadding * WTSR->intsPerHeight), // X descending
		std::vector<uint64_t>(WTSR->chunkSizePadding * WTSR->chunkSizePadding * WTSR->intsPerHeight), // Z ascending
		std::vector<uint64_t>(WTSR->chunkSizePadding * WTSR->chunkSizePadding * WTSR->intsPerHeight), // Z descending
	};

	for (int axis = 0; axis < 6; axis++) { // Iterate all axis ascending and descending 
		// Create the binary plane for each axis
		buildBinaryPlanes(columnFaceMasks[axis], binaryPlanes[axis], axis);

		// Greedy mesh each plane and create planes
		greedyMeshingBinaryPlane(binaryPlanes[axis], axis);
	}
}

void ABinaryChunk::buildBinaryPlanes(const std::vector<uint64_t>& faceMaskColumn, std::vector<uint64_t>& binaryPlane, const int& axis) {
	for (int x = 0; x < WTSR->chunkSizePadding; x++) {
		for (int z = 0; z < WTSR->chunkSizePadding; z++) {
			for (int bitIndex = 0; bitIndex < WTSR->intsPerHeight; bitIndex++) {

				const int columnIndex{ (x * WTSR->chunkSizePadding * WTSR->intsPerHeight) + (z * WTSR->intsPerHeight) + bitIndex }; // VERIFIED! Goes from 0 - 16,383
			
				uint64_t column = faceMaskColumn[columnIndex];  // this goes from 0 - 16,383

				// Remove padding only for X and Z axis 
				if (axis != 0 && axis != 1) {
					// Remove the leftmost bit and the rightmost bit and replace them with 0
					column = (column & ~(1ULL << 63)) &~1ULL;
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
						planeIndex = (y + bitIndex * WTSR->chunkSizePadding) * WTSR->chunkSizePadding + x;
						binaryPlane[planeIndex] |= (1ULL << z);
						break;
					case 2:
					case 3:
					case 4:
					case 5:
						planeRowIndex = y; 
						if (columnIndex > 0) {
							currentPlaneIndex = (columnIndex / WTSR->chunkSizePadding) * WTSR->chunkSizePadding;
							planeIndex = currentPlaneIndex + planeRowIndex;
						} else {
							planeIndex = planeRowIndex;
						}

						binaryPlane[planeIndex] |= (1ULL << columnIndex % WTSR->chunkSizePadding); 
						break;
					}

					// Remove the padding from the plane
					binaryPlane[planeIndex] = (binaryPlane[planeIndex] & ~(1ULL << 63)) & ~1ULL;

					// Clear the position 
					column &= column - 1;
				}
			
			}
		}
	}
}

void ABinaryChunk::greedyMeshingBinaryPlane(std::vector<uint64_t>& planes, const int& axis) {
	for (int row = 0; row < planes.size(); row++) {

		// Removing padding by skipping the first and last row in the plane
		if (row == 0) continue;
		else if (row % WTSR->chunkSizePadding == 0 || row % WTSR->chunkSizePadding == WTSR->chunkSizePadding - 1) continue;

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
				currentPlaneLimit = WTSR->chunkSize; // plane Y max limit is 64

				// Check if the next row can be expanded while in the bounds of the current plane 
				while ((row % WTSR->chunkSizePadding) + width < currentPlaneLimit) {

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
				currentPlaneLimit = WTSR->chunkHeight * WTSR->chunkSize; // plane X and Z max limit is 64x248  (64 for each layer, and 248 layers)

				// Check if the next row can be expanded while in the bounds of the current plane 
				while (row + (width * WTSR->chunkSizePadding) < currentPlaneLimit) {

					// Get the row above the current one
					const int planesIndex = row + (width * WTSR->chunkSizePadding);

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
				voxelX = static_cast<double>(row % WTSR->chunkSizePadding);
				voxelZ = static_cast<double>(y);
				break;
			case 2:
			case 3:
				voxelZ = static_cast<double>(row % WTSR->chunkSizePadding);
				voxelX = static_cast<double>(y);
				break;
			}

			// Height increases with each 64 rows for X and Z
			const double voxelY = row > 0 ? std::floor(static_cast<double>(row / WTSR->chunkSizePadding)) : 0.0;
			voxelPosition1 = { voxelX, voxelZ, voxelY }; // X, Y, Z (height)
			
			// Modify the original voxel position and create the remaining three quad position
			createAllVoxelPositionsFromOriginal(voxelPosition1, voxelPosition2, voxelPosition3, voxelPosition4, width, height, axis);

			// Create the quads
			createQuadAndAddToMeshData(voxelPosition1, voxelPosition2, voxelPosition3, voxelPosition4, width, height, axis);
		}
	}
}

void ABinaryChunk::createQuadAndAddToMeshData(
	const FVector& voxelPosition1,
	const FVector& voxelPosition2,
	const FVector& voxelPosition3,
	const FVector& voxelPosition4,
	const int& width, const int& height,
	const int& axis
	) {
	MeshData.Vertices.Append({
		voxelPosition1 * WTSR->UnrealScale,
		voxelPosition2 * WTSR->UnrealScale,
		voxelPosition3 * WTSR->UnrealScale,
		voxelPosition4 * WTSR->UnrealScale
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

	// Assign different random colors for each vertex; This lets the GPU interpolate the colors
	FVector voxelPositions[] = { voxelPosition1, voxelPosition2, voxelPosition3, voxelPosition4 };
	for (int vertices = 0; vertices < 4; vertices++) {
		int32 colorIndex = FMath::RandRange(0, WTSR->ColorArray[0].Num() - 1);
		int layerIndex = getColorIndexFromVoxelHeight(voxelPositions[vertices]);
		FColor RandomColor = WTSR->ColorArray[layerIndex][colorIndex];
		MeshData.Colors.Add(RandomColor);
	}
}

void ABinaryChunk::spawnTerrainChunkMeshes() {
	Mesh->CreateMeshSection(0, MeshData.Vertices, MeshData.Triangles, MeshData.Normals, MeshData.UV0, MeshData.Colors, TArray<FProcMeshTangent>(), false);

	// Load and apply basic material to the mesh
	UMaterialInterface* Material = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/VoxelBasicMaterial.VoxelBasicMaterial"));
	if (Material) {
		Mesh->SetMaterial(0, Material);
	}
}

void ABinaryChunk::printExecutionTime(Time& start, Time& end, const char* functionName) {
	std::chrono::duration<double, std::milli> duration = end - start;
	UE_LOG(LogTemp, Warning, TEXT("%s() took %f milliseconds to execute."), *FString(functionName), duration.count());
}

// Called when the game starts or when spawned
void ABinaryChunk::BeginPlay() {
	Super::BeginPlay();

	applyPerlinNoiseSettings(continentalness, 0);
	applyPerlinNoiseSettings(erosion, 1);
	applyPerlinNoiseSettings(peaksAndValleys, 2);

	applyDomainWarpSettings(continentalnessDW, 0);
	applyDomainWarpSettings(erosionDW, 1);
	applyDomainWarpSettings(peaksAndValleysDW, 2);

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

