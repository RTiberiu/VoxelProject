// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "..\..\Noise\PerlinNoiseSettings.h"
#include <vector>
#include <chrono>
#include "..\DataStructures\VoxelObjectMeshData.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BinaryChunk.generated.h"

class FastNoiseLite;
class UWorldTerrainSettings;
class UProceduralMeshComponent;
class APerlinNoiseSettings;

UCLASS()
class ABinaryChunk : public AActor {
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ABinaryChunk();

	~ABinaryChunk();

	void SetWorldTerrainSettings(UWorldTerrainSettings* InWorldTerrainSettings);
	void SetPerlinNoiseSettings(APerlinNoiseSettings* InPerlinNoiseSettings);
	void SetComputedMeshData(FVoxelObjectMeshData InMeshData);
	void SetChunkCollision(bool InHasCollision);

	bool HasCollision();
	void UpdateCollision(bool InHasCollision);

private: 
	APerlinNoiseSettings* PerlinNoiseSettingsRef;
	APerlinNoiseSettings*& PNSR = PerlinNoiseSettingsRef;

	UWorldTerrainSettings* WorldTerrainSettingsRef;
	UWorldTerrainSettings*& WTSR = WorldTerrainSettingsRef; // creating an alias for the world terrain settings ref

	FVoxelObjectMeshData meshData;
	FIntPoint chunkLocation;

	bool hasCollision;

	// Create chrono type alias
	using Time = std::chrono::high_resolution_clock::time_point;

	TObjectPtr<UProceduralMeshComponent> Mesh;

	void spawnTerrainChunkMeshes();

	void printExecutionTime(Time& start, Time& end, const char* functionName);

	void printBinary(uint64_t value, int groupSize, const std::string& otherData ="Column value: ");

	void apply3DNoiseToHeightColumn(uint64_t& column, int& x, int& z, int& y, int& bitIndex, const FVector& chunkWorldLocation, int& height);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

};
