#pragma once

#include "..\..\..\Noise\PerlinNoiseSettings.h"

#include "..\..\DataStructures\VoxelObjectLocationData.h"
#include "..\..\DataStructures\VoxelObjectMeshData.h"

#include <vector>
#include <chrono>
#include <random>
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Flower.generated.h"

class UProceduralMeshComponent;
class APerlinNoiseSettings;
class UWorldTerrainSettings;

UCLASS()
class AFlower : public AActor {
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AFlower();
	~AFlower();

	void SetFlowerMeshData(FVoxelObjectMeshData* InMeshData);
	void SetFlowerWorldLocation(FIntPoint InFlowerLocation);

	void SetWorldTerrainSettings(UWorldTerrainSettings* InWorldTerrainSettings);
	void SetPerlinNoiseSettings(APerlinNoiseSettings* InPerlinNoiseSettings);

private:
	APerlinNoiseSettings* PerlinNoiseSettingsRef;
	APerlinNoiseSettings*& PNSR = PerlinNoiseSettingsRef;

	UWorldTerrainSettings* WorldTerrainSettingsRef;
	UWorldTerrainSettings*& WTSR = WorldTerrainSettingsRef;

	FVoxelObjectLocationData FlowerLocationData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flower Settings", meta = (AllowPrivateAccess = "true"))
	FIntPoint FlowerWorldLocation;


	TObjectPtr<UProceduralMeshComponent> Mesh;

	FVoxelObjectMeshData* MeshData; // store vertices, normals, triangles, etc.

	FIntPoint FlowerLocation;
	bool hasCollision;

	// Create chrono type alias
	using Time = std::chrono::high_resolution_clock::time_point;

	void printExecutionTime(Time& start, Time& end, const char* functionName);

	void SpawnFlowerMeshes();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
};
