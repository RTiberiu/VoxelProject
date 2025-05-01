// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <vector>
#include <chrono>
#include "..\DataStructures\VoxelObjectMeshData.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BinaryChunk.generated.h"

class FastNoiseLite;
class UWorldTerrainSettings;
class UProceduralMeshComponent;

UCLASS()
class ABinaryChunk : public AActor {
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ABinaryChunk();

	~ABinaryChunk();

	void SetWorldTerrainSettings(UWorldTerrainSettings* InWorldTerrainSettings);
	void SetComputedMeshData(FVoxelObjectMeshData InMeshData);
	void SetChunkCollision(bool InHasCollision);
	void SetChunkLocation(FIntPoint InChunkLocation);

	bool HasCollision();
	void UpdateCollision(bool InHasCollision);

private: 
	UWorldTerrainSettings* WorldTerrainSettingsRef;
	UWorldTerrainSettings*& WTSR = WorldTerrainSettingsRef; // creating an alias for the world terrain settings ref

	FVoxelObjectMeshData meshData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chunk Settings", meta = (AllowPrivateAccess = "true"))
	FIntPoint chunkLocation;


	bool hasCollision;

	// Create chrono type alias
	using Time = std::chrono::high_resolution_clock::time_point;

	UProceduralMeshComponent* Mesh;

	void spawnTerrainChunkMeshes();

	void printExecutionTime(Time& start, Time& end, const char* functionName);

	void printBinary(uint64_t value, int groupSize, const std::string& otherData ="Column value: ");

	void apply3DNoiseToHeightColumn(uint64_t& column, int& x, int& z, int& y, int& bitIndex, const FVector& chunkWorldLocation, int& height);

	void ApplyCollisionLODColor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;


};
