

#include "Flower.h"
#include <iostream>
#include <array>
#include <bitset>
#include <limits>
#include <string>

#include "..\..\TerrainSettings\WorldTerrainSettings.h"
#include "ProceduralMeshComponent.h"

// Sets default values
AFlower::AFlower() {
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	Mesh = CreateDefaultSubobject<UProceduralMeshComponent>("Mesh");

	Mesh->SetCastShadow(false);

	// By default the Flower shouldn't have collision
	hasCollision = false;

	// Set mesh as root
	SetRootComponent(Mesh);
}

AFlower::~AFlower() {
}

// Set a pointer to the mesh data.
void AFlower::SetFlowerMeshData(FVoxelObjectMeshData* InMeshData) {
	MeshData = InMeshData;
}

void AFlower::SetFlowerWorldLocation(FIntPoint InFlowerLocation) {
	FlowerWorldLocation = InFlowerLocation;
}

void AFlower::SetWorldTerrainSettings(UWorldTerrainSettings* InWorldTerrainSettings) {
	WorldTerrainSettingsRef = InWorldTerrainSettings;
}

void AFlower::SetPerlinNoiseSettings(APerlinNoiseSettings* InPerlinNoiseSettings) {
	PerlinNoiseSettingsRef = InPerlinNoiseSettings;
}

void AFlower::printExecutionTime(Time& start, Time& end, const char* functionName) {
	std::chrono::duration<double, std::milli> duration = end - start;
	UE_LOG(LogTemp, Warning, TEXT("%s() took %f milliseconds to execute."), *FString(functionName), duration.count());
}

void AFlower::SpawnFlowerMeshes() {
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

void AFlower::BeginPlay() {
	Super::BeginPlay();

	SpawnFlowerMeshes();
}
