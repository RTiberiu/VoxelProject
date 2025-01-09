#pragma once

#include "..\..\..\Noise\PerlinNoiseSettings.h"

#include "..\..\DataStructures\VoxelObjectLocationData.h"
#include "..\..\DataStructures\VoxelObjectMeshData.h"

#include <vector>
#include <chrono>
#include <random>
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Grass.generated.h"

class UProceduralMeshComponent;
class APerlinNoiseSettings;
class UWorldTerrainSettings;

UCLASS()
class AGrass : public AActor {
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AGrass();
	~AGrass();

	void SetGrassMeshData(FVoxelObjectMeshData* InMeshData);
	void SetGrassWorldLocation(FIntPoint InGrassLocation);

	void SetWorldTerrainSettings(UWorldTerrainSettings* InWorldTerrainSettings);
	void SetPerlinNoiseSettings(APerlinNoiseSettings* InPerlinNoiseSettings);

private:
	APerlinNoiseSettings* PerlinNoiseSettingsRef;
	APerlinNoiseSettings*& PNSR = PerlinNoiseSettingsRef;

	UWorldTerrainSettings* WorldTerrainSettingsRef;
	UWorldTerrainSettings*& WTSR = WorldTerrainSettingsRef;

	FVoxelObjectLocationData GrassLocationData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grass Settings", meta = (AllowPrivateAccess = "true"))
	FIntPoint GrassWorldLocation;

	TObjectPtr<UProceduralMeshComponent> Mesh;

	FVoxelObjectMeshData* MeshData; // store vertices, normals, triangles, etc.

	FIntPoint GrassLocation;
	bool hasCollision;

	// Create chrono type alias
	using Time = std::chrono::high_resolution_clock::time_point;

	void printExecutionTime(Time& start, Time& end, const char* functionName);

	void SpawnGrassMeshes();



protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaSeconds) override;
};
