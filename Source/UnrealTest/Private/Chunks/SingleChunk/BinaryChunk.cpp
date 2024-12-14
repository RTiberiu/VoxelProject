// Fill out your copyright notice in the Description page of Project Settings.

#include "BinaryChunk.h"
#include "..\TerrainSettings\WorldTerrainSettings.h"
#include "ProceduralMeshComponent.h"

#include <iostream>
#include <array>
#include <bitset>
#include <limits>
#include "..\..\Noise\NoiseLibrary\FastNoiseLite.h"

// Sets default values
ABinaryChunk::ABinaryChunk() {
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	Mesh = CreateDefaultSubobject<UProceduralMeshComponent>("Mesh");

	Mesh->SetCastShadow(false);

	// By default the chunk shouldn't have collision
	hasCollision = false;

	// Set mesh as root
	SetRootComponent(Mesh);
}

ABinaryChunk::~ABinaryChunk() {
}

void ABinaryChunk::SetWorldTerrainSettings(UWorldTerrainSettings* InWorldTerrainSettings) {
	WorldTerrainSettingsRef = InWorldTerrainSettings;
}

void ABinaryChunk::SetPerlinNoiseSettings(APerlinNoiseSettings* InPerlinNoiseSettings) {
	PerlinNoiseSettingsRef = InPerlinNoiseSettings;
}

void ABinaryChunk::SetComputedMeshData(FVoxelObjectMeshData InMeshData) {
	meshData = InMeshData;
}

void ABinaryChunk::SetChunkCollision(bool InHasCollision) {
	hasCollision = InHasCollision;
}

bool ABinaryChunk::HasCollision() {
	return hasCollision;
}

void ABinaryChunk::UpdateCollision(bool InHasCollision) {
	SetChunkCollision(InHasCollision);

	if (hasCollision) {
		// If the chunk has collision, enable it by regenerating the mesh
		Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		Mesh->SetCollisionResponseToAllChannels(ECR_Block);
		Mesh->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);

		Mesh->CreateMeshSection(0, meshData.Vertices, meshData.Triangles, meshData.Normals, meshData.UV0, meshData.Colors, TArray<FProcMeshTangent>(), hasCollision);
	} else {
		// If the chunk does not have collision, disable it by updating the mesh
		Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Mesh->UpdateMeshSection(0, meshData.Vertices, meshData.Normals, meshData.UV0, meshData.Colors, TArray<FProcMeshTangent>());
	}
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

void ABinaryChunk::spawnTerrainChunkMeshes() {	

	Mesh->CreateMeshSection(0, meshData.Vertices, meshData.Triangles, meshData.Normals, meshData.UV0, meshData.Colors, TArray<FProcMeshTangent>(), hasCollision);

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

void ABinaryChunk::printExecutionTime(Time& start, Time& end, const char* functionName) {
	std::chrono::duration<double, std::milli> duration = end - start;
	UE_LOG(LogTemp, Warning, TEXT("%s() took %f milliseconds to execute."), *FString(functionName), duration.count());
}

// Called when the game starts or when spawned
void ABinaryChunk::BeginPlay() {
	Super::BeginPlay();

	//Time start = std::chrono::high_resolution_clock::now();

	// createBinarySolidColumnsYXZ();

	//Time end = std::chrono::high_resolution_clock::now();

	// printExecutionTime(start, end, "createBinarySolidColumnsYXZ");

	//start = std::chrono::high_resolution_clock::now();

	// createTerrainMeshesData();

	//end = std::chrono::high_resolution_clock::now();

	// printExecutionTime(start, end, "createTerrainMeshesData");

	//start = std::chrono::high_resolution_clock::now();

	spawnTerrainChunkMeshes();

	//end = std::chrono::high_resolution_clock::now();

	// Removing the mesh data after the chunk is spawned
	// meshData = FChunkMeshData(); 

	// printExecutionTime(start, end, "spawnTerrainChunkMeshes");

	// Testing generating simple cube
	// testingMeshingCreation();
	// generateChunkMeshes();

}

