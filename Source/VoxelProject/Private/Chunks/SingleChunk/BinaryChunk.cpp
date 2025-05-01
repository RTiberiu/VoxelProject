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

void ABinaryChunk::SetComputedMeshData(FVoxelObjectMeshData InMeshData) {
	meshData = InMeshData;
}

void ABinaryChunk::SetChunkCollision(bool InHasCollision) {
	hasCollision = InHasCollision;
}

void ABinaryChunk::SetChunkLocation(FIntPoint InChunkLocation) {
	chunkLocation = InChunkLocation;
}

bool ABinaryChunk::HasCollision() {
	return hasCollision;
}

void ABinaryChunk::UpdateCollision(bool InHasCollision) {
	SetChunkCollision(InHasCollision);

	// Only runs in debug mode, and changes the color of the mesh based on collision
	ApplyCollisionLODColor();

	if (hasCollision) {
		// If the chunk has collision, enable it by regenerating the mesh
		Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		Mesh->SetCollisionResponseToAllChannels(ECR_Block);
		Mesh->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
		Mesh->SetCanEverAffectNavigation(false);

		Mesh->CreateMeshSection(0, meshData.Vertices, meshData.Triangles, meshData.Normals, meshData.UV0, meshData.Colors, TArray<FProcMeshTangent>(), hasCollision);
	} else {
		// If the chunk does not have collision, disable it by updating the mesh
		Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Mesh->UpdateMeshSection(0, meshData.Vertices, meshData.Normals, meshData.UV0, meshData.Colors, TArray<FProcMeshTangent>());
		Mesh->SetCanEverAffectNavigation(false);

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

void ABinaryChunk::spawnTerrainChunkMeshes() {	
	// Only runs in debug mode, and changes the color of the mesh based on collision
	ApplyCollisionLODColor();

	Mesh->CreateMeshSection(0, meshData.Vertices, meshData.Triangles, meshData.Normals, meshData.UV0, meshData.Colors, TArray<FProcMeshTangent>(), hasCollision);

	Mesh->SetCastShadow(WTSR->ChunkShadow);

	// Set up simplified collision
	if (hasCollision) {
		Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		Mesh->SetCollisionResponseToAllChannels(ECR_Block);
		Mesh->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
		Mesh->SetCanEverAffectNavigation(true);
	}

	Mesh->SetCanEverAffectNavigation(true); // TESTING 
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

	spawnTerrainChunkMeshes();

}

void ABinaryChunk::ApplyCollisionLODColor() {
	if (!WTSR->ShowChunkCollisionLOD) {
		return;
	}

	const FColor CollisionColor = hasCollision
		? FColor(252, 65, 3)
		: FColor(254, 255, 232);

	for (FColor& VertexColor : meshData.Colors) {
		VertexColor = CollisionColor;
	}
}
