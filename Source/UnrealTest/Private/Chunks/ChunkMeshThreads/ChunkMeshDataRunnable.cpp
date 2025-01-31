// Fill out your copyright notice in the Description page of Project Settings.

#include "ChunkMeshDataRunnable.h"
#include "..\TerrainSettings\WorldTerrainSettings.h"
#include <iostream>
#include <array>
#include <bitset>
#include <limits>
#include "..\..\Noise\NoiseLibrary\FastNoiseLite.h"
#include "..\ChunkData\ChunkLocationData.h"

//#include "..\ChunkData\ObjectMeshData.h"


ChunkMeshDataRunnable::ChunkMeshDataRunnable(FVoxelObjectLocationData InChunkLocationData, UWorldTerrainSettings* InWorldTerrainSettingsRef, UChunkLocationData* InChunkLocationDataRef, APerlinNoiseSettings* InPerlinNoiseSettingsRef) : ChunkLocationData(InChunkLocationData) {
	WorldTerrainSettingsRef = InWorldTerrainSettingsRef;
	ChunkLocationDataRef = InChunkLocationDataRef;
	PerlinNoiseSettingsRef = InPerlinNoiseSettingsRef;
}

ChunkMeshDataRunnable::~ChunkMeshDataRunnable() {
}

bool ChunkMeshDataRunnable::Init() {
	isRunning.AtomicSet(true);
	isTaskComplete.AtomicSet(false);
	return true;
}

uint32 ChunkMeshDataRunnable::Run() {
	while (isRunning) {

		Time start = std::chrono::high_resolution_clock::now();

		createBinarySolidColumnsYXZ();
		createTerrainMeshesData();

		Time end = std::chrono::high_resolution_clock::now();

		WTSR->chunkSpawnTime += end - start;
		WTSR->chunksMeshCounter++;

		WTSR->UpdateChunkSemaphore->Acquire(); // TODO This might need to be changed to a different semaphore
		CLDR->addMeshDataForPosition(ChunkLocationData, TemporaryMeshData);
		TemporaryMeshData = FVoxelObjectMeshData(); // TODO This might not need to be cleared
		WTSR->UpdateChunkSemaphore->Release();

		isTaskComplete.AtomicSet(true);
		isRunning.AtomicSet(false);
	}
	return 0;
}

void ChunkMeshDataRunnable::Stop() {
	isRunning.AtomicSet(false);
}

void ChunkMeshDataRunnable::Exit() {
}

FThreadSafeBool ChunkMeshDataRunnable::IsTaskComplete() const {
	return isTaskComplete;
}

void ChunkMeshDataRunnable::SetChunkLocationData(UChunkLocationData* InChunkLocationData) {
	ChunkLocationDataRef = InChunkLocationData;
}

void ChunkMeshDataRunnable::SetWorldTerrainSettings(UWorldTerrainSettings* InWorldTerrainSettings) {
	WorldTerrainSettingsRef = InWorldTerrainSettings;
}

void ChunkMeshDataRunnable::SetPerlinNoiseSettings(APerlinNoiseSettings* InPerlinNoiseSettingsRef) {
	PerlinNoiseSettingsRef = InPerlinNoiseSettingsRef;
}

void ChunkMeshDataRunnable::printExecutionTime(Time& start, Time& end, const char* functionName) {
	std::chrono::duration<double, std::milli> duration = end - start;
	UE_LOG(LogTemp, Warning, TEXT("%s() took %d seconds, %d milliseconds to execute."), *FString(functionName),
		static_cast<int>((duration.count() / 1000)) % 60,
		static_cast<int>(fmod(duration.count(), 1000)));
}

void ChunkMeshDataRunnable::createBinarySolidColumnsYXZ() {
	const FVector chunkWorldLocation = ChunkLocationData.ObjectPosition;

	// Set the chunk values to air for all 3 axis (Y, X, Z)
	binaryChunk.yBinaryColumn = std::vector<uint64_t>(WTSR->chunkSizePadding * WTSR->chunkSizePadding * WTSR->intsPerHeight, 0);
	binaryChunk.xBinaryColumn = std::vector<uint64_t>(WTSR->chunkSizePadding * WTSR->chunkSizePadding * WTSR->intsPerHeight, 0);
	binaryChunk.zBinaryColumn = std::vector<uint64_t>(WTSR->chunkSizePadding * WTSR->chunkSizePadding * WTSR->intsPerHeight, 0);
	
	// Vegetation spawn area 
	int vegetationXZLimit = WTSR->chunkSizePadding - WTSR->chunkSize;
	
	// Surface voxel points that will be used for pathfinding
	TArray<int> surfaceVoxelPoints;

	// Holding the correct noise settings for each noise type
	const FNoiseMapSettings& continentalnessSettings = PNSR->noiseMapSettings[0];
	const FNoiseMapSettings& erosionSettings = PNSR->noiseMapSettings[1];
	const FNoiseMapSettings& peaksAndValleysSettings = PNSR->noiseMapSettings[2];

	// Loop over the chunk dimensions (X, Y, Z)
	for (int x = 0; x < WTSR->chunkSizePadding; x++) {
		for (int z = 0; z < WTSR->chunkSizePadding; z++) {

			// Looping over the different octaves to get the final height
			int amplitude{ 0 };

			// Getting perlin noise position, adjusted to the Unreal Engine grid system 
			float noisePositionX = static_cast<float>((x * WTSR->UnrealScale + chunkWorldLocation.X) / WTSR->UnrealScale);
			float noisePositionZ = static_cast<float>((z * WTSR->UnrealScale + chunkWorldLocation.Y) / WTSR->UnrealScale);

			// Apply domain warp to each noise map and get value
			WTSR->ApplyDomainWarpToCoords(noisePositionX, noisePositionZ, WTSR->continentalnessDW);
			WTSR->ApplyDomainWarpToCoords(noisePositionX, noisePositionZ, WTSR->erosionDW);
			WTSR->ApplyDomainWarpToCoords(noisePositionX, noisePositionZ, WTSR->peaksAndValleysDW);

			const float continentalnessVal = WTSR->GetNoiseAtCoords(noisePositionX, noisePositionZ, WTSR->continentalness);
			const float erosionVal = WTSR->GetNoiseAtCoords(noisePositionX, noisePositionZ, WTSR->erosion);
			const float peaksAndValleysVal = WTSR->GetNoiseAtCoords(noisePositionX, noisePositionZ, WTSR->peaksAndValleys);

			// Adding multiple splines to the continentalness (inland)
			float continentalnessHeight = 0.0f;
			float erosionHeight = 0.0f;

			float amplitudePV = 0.0f;

			if (continentalnessVal <= 1.5) {
				float normalizedContNoise = (continentalnessVal - 0.5f) / 1.0f;
				const float amplitudeContinent = std::lerp(10, 150, normalizedContNoise);

				continentalnessHeight = continentalnessVal * amplitudeContinent;

				// Adding erosion 
				const float amplitudeErosion = erosionSettings.Amplitudes[0];
				float normalizedErosNoise = (erosionVal - 0.5f) / 1.0f;
				erosionHeight = erosionVal * amplitudeErosion;

				// Adding peaks and valleys
				if (erosionVal <= 1) {
					amplitudePV = peaksAndValleysSettings.Amplitudes[0];
				} else if (erosionVal <= 1.5) {
					amplitudePV = peaksAndValleysSettings.Amplitudes[1];
				} else {
					amplitudePV = peaksAndValleysSettings.Amplitudes[2];
				}
			} else {
				continentalnessHeight = 150;

				// Adding erosion 
				const float amplitudeErosion = erosionSettings.Amplitudes[1];
				erosionHeight = erosionVal * amplitudeErosion;

				// Adding peaks and valleys
				if (erosionVal <= 1) {
					amplitudePV = peaksAndValleysSettings.Amplitudes[3];
				} else if (erosionVal <= 1.5) {
					amplitudePV = peaksAndValleysSettings.Amplitudes[4];
				} else {
					amplitudePV = peaksAndValleysSettings.Amplitudes[5];
				}
			}

			// Adding multiple splines to the erosion (height)
			const float peaksAndValleysHeight = peaksAndValleysVal * amplitudePV;

			const int combinedNoiseHeight = static_cast<int>(std::floor(continentalnessHeight + erosionHeight + peaksAndValleysHeight));

			// Ensuring height remains between chunk borders
			int height = std::clamp(combinedNoiseHeight, 0, static_cast<int>(WTSR->chunkHeight));

			// Add the voxel point to the surface voxel points array
			surfaceVoxelPoints.Add(height);

			// Spawn some vegetation at current voxel point
			attemptToSpawnVegetationAtLocation(x, z, height, vegetationXZLimit, chunkWorldLocation);

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

	// Add the surface voxel points to the chunk location data
	CLDR->AddSurfaceVoxelPointsForChunk(ChunkLocationData.ObjectWorldCoords, surfaceVoxelPoints, surfaceAvoidPositions);

}

void ChunkMeshDataRunnable::faceCullingBinaryColumnsYXZ(std::vector<std::vector<uint64_t>>& columnFaceMasks) {
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

void ChunkMeshDataRunnable::createTerrainMeshesData() {
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

void ChunkMeshDataRunnable::buildBinaryPlanes(const std::vector<uint64_t>& faceMaskColumn, std::vector<uint64_t>& binaryPlane, const int& axis) {
	for (int x = 0; x < WTSR->chunkSizePadding; x++) {
		for (int z = 0; z < WTSR->chunkSizePadding; z++) {
			for (int bitIndex = 0; bitIndex < WTSR->intsPerHeight; bitIndex++) {

				const int columnIndex{ (x * WTSR->chunkSizePadding * WTSR->intsPerHeight) + (z * WTSR->intsPerHeight) + bitIndex }; // VERIFIED! Goes from 0 - 16,383

				uint64_t column = faceMaskColumn[columnIndex];  // this goes from 0 - 16,383

				// Remove padding only for X and Z axis 
				if (axis != 0 && axis != 1) {
					// Remove the leftmost bit and the rightmost bit and replace them with 0
					column = (column & ~(1ULL << 63)) & ~1ULL;
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

void ChunkMeshDataRunnable::greedyMeshingBinaryPlane(std::vector<uint64_t>& planes, const int& axis) {
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

// Modify the original voxel position and create the remaining three quad position
void ChunkMeshDataRunnable::createAllVoxelPositionsFromOriginal(FVector& voxelPosition1, FVector& voxelPosition2, FVector& voxelPosition3, FVector& voxelPosition4, const int& width, const int& height, const int& axis) {

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

void ChunkMeshDataRunnable::createQuadAndAddToMeshData(const FVector& voxelPosition1, const FVector& voxelPosition2, const FVector& voxelPosition3, const FVector& voxelPosition4, const int& height, const int& width, const int& axis) {
	TemporaryMeshData.Vertices.Append({
		voxelPosition1 * WTSR->UnrealScale,
		voxelPosition2 * WTSR->UnrealScale,
		voxelPosition3 * WTSR->UnrealScale,
		voxelPosition4 * WTSR->UnrealScale
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
	int layerIndex = getColorIndexFromVoxelHeight(static_cast<int>(voxelPosition1.Z));
	FColor layerColor = WTSR->ChunkColorArray[layerIndex];

	TemporaryMeshData.Colors.Append({
		layerColor,
		layerColor,
		layerColor,
		layerColor
		});
}

int ChunkMeshDataRunnable::getColorIndexFromVoxelHeight(const int& height) {
	const int colorIndex = FMath::Clamp(height / WTSR->LayerHeight, 0, WTSR->ColorLayers - 1);
	return colorIndex;
}

void ChunkMeshDataRunnable::attemptToSpawnVegetationAtLocation(const int& x, const int& z, const int& height, const int& vegetationXZLimit, const FVector& chunkWorldLocation) {

	// Return early if the point is not on grass
	const int colorIndex = getColorIndexFromVoxelHeight(height);
	bool pointNotOnGrass = colorIndex < WTSR->GrassColorStartIndex || colorIndex > WTSR->GrassColorEndIndex;
	if (pointNotOnGrass) {
		return;
	}

	float spawnVegetationChance = FMath::RandRange(0.0f, 1.0f);
	bool isVegetationInsideChunk = x >= vegetationXZLimit && z >= vegetationXZLimit;

	if (isVegetationInsideChunk) {
		FVoxelObjectLocationData vegetationSpawnPosition;

		// TODO Extract the X and Z calculations into a different functions or simplify the code
		if (spawnVegetationChance < WTSR->TreeSpawnChance) {
			float treeX = (x * WTSR->UnrealScale + chunkWorldLocation.X - ((WTSR->TreeSize * WTSR->TreeScale) / 2) + WTSR->UnrealScale / 2) - WTSR->TreeScale / 2;
			float treeZ = (z * WTSR->UnrealScale + chunkWorldLocation.Y - ((WTSR->TreeSize * WTSR->TreeScale) / 2) + WTSR->UnrealScale / 2) - WTSR->TreeScale / 2;
			vegetationSpawnPosition.ObjectPosition = FVector(treeX, treeZ, height * WTSR->UnrealScale);
			vegetationSpawnPosition.ObjectWorldCoords = ChunkLocationData.ObjectWorldCoords;

			CLDR->addTreeSpawnPosition(vegetationSpawnPosition);

			// Add position to be avoided in the pathfinding
			surfaceAvoidPositions.Add(FVector2D(x, z));

		} else if (spawnVegetationChance < WTSR->FlowerSpawnChance) {
			// Reduce the flower spawn levels by 2 
			bool isOnReducedGrassLevel = colorIndex > WTSR->GrassColorEndIndex - 2;
			if (isOnReducedGrassLevel) return;

			float flowerX = (x * WTSR->UnrealScale + chunkWorldLocation.X - ((WTSR->FlowerSize * WTSR->FlowerScale) / 2) + WTSR->UnrealScale / 2) - WTSR->FlowerScale / 2;
			float flowerZ = (z * WTSR->UnrealScale + chunkWorldLocation.Y - ((WTSR->FlowerSize * WTSR->FlowerScale) / 2) + WTSR->UnrealScale / 2) - WTSR->FlowerScale / 2;
			vegetationSpawnPosition.ObjectPosition = FVector(flowerX, flowerZ, height * WTSR->UnrealScale);
			vegetationSpawnPosition.ObjectWorldCoords = ChunkLocationData.ObjectWorldCoords;

			CLDR->addFlowerSpawnPosition(vegetationSpawnPosition);
		} else if (spawnVegetationChance < WTSR->GrassSpawnChance) {
			// Reduce the grass spawn levels by 2 
			bool isOnReducedGrassLevel = colorIndex > WTSR->GrassColorEndIndex - 2;
			if (isOnReducedGrassLevel) return;

			float grassX = (x * WTSR->UnrealScale + chunkWorldLocation.X - ((WTSR->GrassSize * WTSR->GrassScale) / 2) + WTSR->UnrealScale / 2) - WTSR->GrassScale / 2;
			float grassZ = (z * WTSR->UnrealScale + chunkWorldLocation.Y - ((WTSR->GrassSize * WTSR->GrassScale) / 2) + WTSR->UnrealScale / 2) - WTSR->GrassScale / 2;
			vegetationSpawnPosition.ObjectPosition = FVector(grassX, grassZ, height * WTSR->UnrealScale);
			vegetationSpawnPosition.ObjectWorldCoords = ChunkLocationData.ObjectWorldCoords;

			CLDR->addGrassSpawnPosition(vegetationSpawnPosition);
		} else if (spawnVegetationChance < WTSR->NPCSpawnChance) {
			// TODO Adjust the X and Z, since this is just a copy of the tree calculation
			float npcX = (x * WTSR->UnrealScale + chunkWorldLocation.X - ((WTSR->TreeSize * WTSR->TreeScale) / 2) + WTSR->UnrealScale / 2) - WTSR->TreeScale / 2;
			float npcZ = (z * WTSR->UnrealScale + chunkWorldLocation.Y - ((WTSR->TreeSize * WTSR->TreeScale) / 2) + WTSR->UnrealScale / 2) - WTSR->TreeScale / 2;

			vegetationSpawnPosition.ObjectPosition = FVector(npcX, npcZ, height * WTSR->UnrealScale);
			vegetationSpawnPosition.ObjectWorldCoords = ChunkLocationData.ObjectWorldCoords;

			CLDR->addNPCSpawnPosition(vegetationSpawnPosition);
		}
	}
}

