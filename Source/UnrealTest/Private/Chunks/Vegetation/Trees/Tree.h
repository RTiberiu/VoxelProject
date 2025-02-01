#pragma once

#include "..\..\..\Noise\PerlinNoiseSettings.h"

#include "..\..\DataStructures\VoxelObjectLocationData.h"
#include "..\..\DataStructures\VoxelObjectMeshData.h"

#include <vector>
#include <chrono>
#include <random>
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Tree.generated.h"

class UProceduralMeshComponent;
class UWorldTerrainSettings;

UCLASS()
class ATree : public AActor {
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ATree();
	~ATree();

	void SetTreeCollision(bool InHasCollision);
	void UpdateCollision(bool InHasCollision);
	void SetTreeMeshData(FVoxelObjectMeshData* InMeshData);
	bool HasCollision();
	void SetTreeWorldLocation(FIntPoint InTreeLocation);

	void SetWorldTerrainSettings(UWorldTerrainSettings* InWorldTerrainSettings);
private:
	UWorldTerrainSettings* WorldTerrainSettingsRef;
	UWorldTerrainSettings*& WTSR = WorldTerrainSettingsRef;

	FVoxelObjectLocationData TreeLocationData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tree Settings", meta = (AllowPrivateAccess = "true"))
	FIntPoint treeWorldLocation;

	UProceduralMeshComponent* Mesh;

	FVoxelObjectMeshData* MeshData; // store vertices, normals, triangles, etc.

	FIntPoint treeLocation;
	bool hasCollision;

	// Create chrono type alias
	using Time = std::chrono::high_resolution_clock::time_point;

	void printExecutionTime(Time& start, Time& end, const char* functionName);

	// TODO This might be combined or something. They are just the modified version from the ChunkMeshDataRunnable.cpp
	void printBinary(uint64_t value, int groupSize, const std::string& otherData);

	void SpawnTreeMeshes();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
};
