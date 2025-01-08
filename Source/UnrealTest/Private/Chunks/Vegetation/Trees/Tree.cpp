

#include "Tree.h"
#include <iostream>
#include <array>
#include <bitset>
#include <limits>
#include <string>

#include "..\..\TerrainSettings\WorldTerrainSettings.h"
#include "ProceduralMeshComponent.h"

// Sets default values
ATree::ATree() {
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	Mesh = CreateDefaultSubobject<UProceduralMeshComponent>("Mesh");

	Mesh->SetCastShadow(false);

	// By default the tree shouldn't have collision
	hasCollision = false;

	// Set mesh as root
	SetRootComponent(Mesh);
}

ATree::~ATree() {
}

void ATree::SetTreeCollision(bool InHasCollision) {
	hasCollision = InHasCollision;
}

void ATree::UpdateCollision(bool InHasCollision) {
	SetTreeCollision(InHasCollision);

	if (InHasCollision) {
		// If the tree has collision, enable it by regenerating the mesh
		Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		Mesh->SetCollisionResponseToAllChannels(ECR_Block);
		Mesh->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);

		Mesh->CreateMeshSection(0, MeshData->Vertices, MeshData->Triangles, MeshData->Normals, MeshData->UV0, MeshData->Colors, TArray<FProcMeshTangent>(), InHasCollision);
	} else {
		// If the tree does not have collision, disable it by updating the mesh
		Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Mesh->UpdateMeshSection(0, MeshData->Vertices, MeshData->Normals, MeshData->UV0, MeshData->Colors, TArray<FProcMeshTangent>());
	}
}

// Set a pointer to the mesh data.
void ATree::SetTreeMeshData(FVoxelObjectMeshData* InMeshData) {
	MeshData = InMeshData;
}

bool ATree::HasCollision() {
	return hasCollision;
}

void ATree::SetTreeWorldLocation(FIntPoint InTreeLocation) {
	treeWorldLocation = InTreeLocation;
}

void ATree::SetWorldTerrainSettings(UWorldTerrainSettings* InWorldTerrainSettings) {
	WorldTerrainSettingsRef = InWorldTerrainSettings;
}

void ATree::SetPerlinNoiseSettings(APerlinNoiseSettings* InPerlinNoiseSettings) {
	PerlinNoiseSettingsRef = InPerlinNoiseSettings;
}

void ATree::printExecutionTime(Time& start, Time& end, const char* functionName) {
	std::chrono::duration<double, std::milli> duration = end - start;
	UE_LOG(LogTemp, Warning, TEXT("%s() took %f milliseconds to execute."), *FString(functionName), duration.count());
}

// Debugging function that prints a 64-bit integer in groups
void ATree::printBinary(uint64_t value, int groupSize, const std::string& otherData) {
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

void ATree::SpawnTreeMeshes() {
	Mesh->CreateMeshSection(0, MeshData->Vertices, MeshData->Triangles, MeshData->Normals, MeshData->UV0, MeshData->Colors, TArray<FProcMeshTangent>(), hasCollision);

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

void ATree::BeginPlay() {
	Super::BeginPlay();

	SpawnTreeMeshes();
}
