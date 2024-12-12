// Fill out your copyright notice in the Description page of Project Settings.

#include "WorldTerrainSettings.h"
#include "..\..\Noise\PerlinNoiseSettings.h"
#include "..\..\Noise\NoiseLibrary\FastNoiseLite.h"
#include "..\..\Utils\Semaphore\FairSemaphore.h"


UWorldTerrainSettings::UWorldTerrainSettings() : 
	UpdateChunkSemaphore(new FairSemaphore(1)),
	PlayerPositionSemaphore(new FairSemaphore(1)),
	MapSemaphore(new FairSemaphore(1)),
	DrawDistanceSemaphore(new FairSemaphore(1)) {
	// Reserve memory for double the draw distance for X and Z 
	SpawnedChunksMap.Reserve(DrawDistance * DrawDistance * 2); 
}

UWorldTerrainSettings::~UWorldTerrainSettings() {
	// Clean up semaphores
	delete UpdateChunkSemaphore;
	delete PlayerPositionSemaphore;
	delete MapSemaphore;
	delete DrawDistanceSemaphore;
}

void UWorldTerrainSettings::AddChunkToMap(const FIntPoint& ChunkCoordinates, AActor* ChunkActor) {
	MapSemaphore->Acquire();
	SpawnedChunksMap.Add(ChunkCoordinates, ChunkActor);
	MapSemaphore->Release();
}

AActor* UWorldTerrainSettings::GetAndRemoveChunkFromMap(const FIntPoint& ChunkCoordinates) {
	MapSemaphore->Acquire();
	// ValidateSpawnedChunksMap();
	AActor* RemovedChunk = nullptr;
	if (SpawnedChunksMap.Contains(ChunkCoordinates)) {
		RemovedChunk = SpawnedChunksMap.FindAndRemoveChecked(ChunkCoordinates);
	}
	// ValidateSpawnedChunksMap();
	MapSemaphore->Release();
	return RemovedChunk;
}

AActor* UWorldTerrainSettings::GetNextChunkFromMap() {
	MapSemaphore->Acquire();
	AActor* RemovedChunk = nullptr;
	FIntPoint keyToRemove = FIntPoint::ZeroValue;

	if (SpawnedChunksMap.Num() > 0) {
		// Get the first item in the map
		for (TPair<FIntPoint, AActor*>& Elem : SpawnedChunksMap) {
			RemovedChunk = Elem.Value;
			keyToRemove = Elem.Key;
			break; 
		}
		// Remove the retrived chunk from the map 
		SpawnedChunksMap.Remove(keyToRemove);
	}
	MapSemaphore->Release();
	return RemovedChunk;
}

int UWorldTerrainSettings::GetMapSize() {
	MapSemaphore->Acquire();
	int MapSize = SpawnedChunksMap.Num();
	MapSemaphore->Release();
	return MapSize;
}

void UWorldTerrainSettings::updateInitialPlayerPosition(FVector newPosition) {
	PlayerPositionSemaphore->Acquire();
	playerInitialPosition = newPosition;
	PlayerPositionSemaphore->Release();
}

FVector UWorldTerrainSettings::getInitialPlayerPosition() {
	PlayerPositionSemaphore->Acquire();
	FVector Position = playerInitialPosition;
	PlayerPositionSemaphore->Release();
	return Position;
}

void UWorldTerrainSettings::EmptyChunkMap() {
	MapSemaphore->Acquire();
	SpawnedChunksMap.Empty();
	MapSemaphore->Release();
}

// Get a read-only version of the map
const TMap<FIntPoint, AActor*>& UWorldTerrainSettings::GetSpawnedChunksMap() const {
	return SpawnedChunksMap;
}

bool UWorldTerrainSettings::isActorPresentInMap(AActor* actor) {
	for (const TPair<FIntPoint, AActor*>& Pair : SpawnedChunksMap) {
		if (Pair.Value == actor) {
			return true;
		}
	}
	return false;
}

void UWorldTerrainSettings::printMapElements(FString message) {
	int testCounter{ 0 };
	FString keysString = message + " Map count: " + FString::FromInt(SpawnedChunksMap.Num()) + ": \n";

	for (const TPair<FIntPoint, AActor*>& Pair : SpawnedChunksMap) {

		if (!Pair.Value) {
			UE_LOG(LogTemp, Error, TEXT("Null actor found in SpawnedChunksMap at coordinates X:%d, Z:%d"), Pair.Key.X, Pair.Key.Y);
			continue;
		}

		keysString += FString::Printf(TEXT("\tX:%d, Z:%d "), Pair.Key.X, Pair.Key.Y);
		testCounter++;

		if (testCounter == DrawDistance * 2) {
			keysString += TEXT("\n");
			testCounter = 0;
		}
	}
	UE_LOG(LogTemp, Warning, TEXT("%s"), *keysString);
}

void UWorldTerrainSettings::ValidateSpawnedChunksMap() {
	CheckForDuplicateActorPointers();

	CheckIfActorIsNullOrPendingKill();

	CheckNumberOfElements();
}

// Testing method that checks for duplicated Chunk (AActor) pointers  in SpawnChunkMap
void UWorldTerrainSettings::CheckForDuplicateActorPointers() {
	// Create a set to track encountered actors
	TSet<AActor*> SeenActors;

	// Iterate over the map values
	for (const TPair<FIntPoint, AActor*>& Pair : SpawnedChunksMap) {
		AActor* Actor = Pair.Value;

		// Check if the actor is already in the set
		if (SeenActors.Contains(Actor)) {
			// Duplicate found
			UE_LOG(LogTemp, Error, TEXT("Duplicate actor detected: %s"), *Actor->GetName());
		}

		// Add the actor to the set
		SeenActors.Add(Actor);
	}
}

void UWorldTerrainSettings::CheckIfActorIsNullOrPendingKill() {
	for (TPair<FIntPoint, AActor*>& Elem : SpawnedChunksMap) {
		if (!IsValid(Elem.Value) || Elem.Value->IsPendingKillPending()) {
			UE_LOG(LogTemp, Error, TEXT("CheckIfActorIsNullOrPendingKill(): Invalid chunk at coordinates: (%d, %d)"), Elem.Key.X, Elem.Key.Y);
		}
	}
}

void UWorldTerrainSettings::CheckNumberOfElements() {
	if (SpawnedChunksMap.Num() < 100) {
		printMapElements("CheckNumberOfElements(): ");
		UE_LOG(LogTemp, Error, TEXT("Map has less than 100 items!: Map size: %d"), SpawnedChunksMap.Num());
	} else if (SpawnedChunksMap.Num() > 101) {
		printMapElements("CheckNumberOfElements(): ");
		UE_LOG(LogTemp, Error, TEXT("Map has more than 100 items!: Map size: %d"), SpawnedChunksMap.Num());
	}
}

void UWorldTerrainSettings::initializePerlinNoise(TObjectPtr<FastNoiseLite>& noise) {
	noise = new FastNoiseLite();
	noise->SetNoiseType(FastNoiseLite::NoiseType_Perlin);
	noise->SetFractalType(FastNoiseLite::FractalType_FBm);
}

void UWorldTerrainSettings::applyDomainWarpSettings(TObjectPtr<FastNoiseLite>& domainWarp, const int& settingsIndex) {
	// Adding domain warp settings
	domainWarp->SetDomainWarpAmp(PNSR->noiseMapSettings[settingsIndex].DomainWarpAmp);
	domainWarp->SetFrequency(PNSR->noiseMapSettings[settingsIndex].DomainWarpFrequencies);
	domainWarp->SetFractalOctaves(PNSR->noiseMapSettings[settingsIndex].DomainWarpOctaves);
	domainWarp->SetFractalLacunarity(PNSR->noiseMapSettings[settingsIndex].DomainWarpLacunarity);
	domainWarp->SetFractalGain(PNSR->noiseMapSettings[settingsIndex].DomainWarpGain);
}

// Use the settingsIndex to apply the correct noise settings from PerlinNoiseSettings.cpp
void UWorldTerrainSettings::applyPerlinNoiseSettings(TObjectPtr<FastNoiseLite>& noise, const int& settingsIndex) {
	// Set perlin noise settings
	noise->SetFractalOctaves(PNSR->noiseMapSettings[settingsIndex].Octaves);
	noise->SetFrequency(PNSR->noiseMapSettings[settingsIndex].Frequencies);
	noise->SetFractalLacunarity(PNSR->noiseMapSettings[settingsIndex].Lacunarity);
	noise->SetFractalGain(PNSR->noiseMapSettings[settingsIndex].Gain);
	noise->SetFractalWeightedStrength(PNSR->noiseMapSettings[settingsIndex].WeightedStrength);
}

// Use the settingsIndex to apply the correct noise settings from PerlinNoiseSettings.cpp
void UWorldTerrainSettings::initializeDomainWarpNoise(TObjectPtr<FastNoiseLite>& domainWarp) {
	domainWarp = new FastNoiseLite();
	domainWarp->SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
	domainWarp->SetFractalType(FastNoiseLite::FractalType_DomainWarpProgressive);
}

// Set the perlin noise data reference and iniaitlize the FastNoise objects with their settings
void UWorldTerrainSettings::SetPerlinNoiseSettings(APerlinNoiseSettings* InPerlinNoiseSettings) {
	PerlinNoiseSettingsRef = InPerlinNoiseSettings;

	// Declaring the 3 noise objects used to create the terrain
	initializePerlinNoise(continentalness);
	initializePerlinNoise(erosion);
	initializePerlinNoise(peaksAndValleys);

	// Declaring the 3 noise domain warp objects to modify the terrain
	initializeDomainWarpNoise(continentalnessDW);
	initializeDomainWarpNoise(erosionDW);
	initializeDomainWarpNoise(peaksAndValleysDW);

	// Apply settings
	applyPerlinNoiseSettings(continentalness, 0);
	applyPerlinNoiseSettings(erosion, 1);
	applyPerlinNoiseSettings(peaksAndValleys, 2);

	applyDomainWarpSettings(continentalnessDW, 0);
	applyDomainWarpSettings(erosionDW, 1);
	applyDomainWarpSettings(peaksAndValleysDW, 2);
}

void UWorldTerrainSettings::ApplyDomainWarpToCoords(float& noisePositionX, float& noisePositionZ, TObjectPtr<FastNoiseLite> noise) {
	noise->DomainWarp(noisePositionX, noisePositionZ);
}

float UWorldTerrainSettings::GetNoiseAtCoords(float& noisePositionX, float& noisePositionZ, TObjectPtr<FastNoiseLite> noise) {
	return noise->GetNoise(noisePositionX, noisePositionZ) + 1;;
}
