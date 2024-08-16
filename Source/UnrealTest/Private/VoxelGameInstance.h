// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "PerlinNoiseSettings.h"
#include "WorldTerrainSettings.h"
#include "ChunkLocationData.h"
#include "ChunkWorld.h"

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "VoxelGameInstance.generated.h"

UCLASS()
class UVoxelGameInstance : public UGameInstance {
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	UVoxelGameInstance();

	void Shutdown();

private:
	UWorldTerrainSettings* worldTerrainSettings;
	UChunkLocationData* chunkLocationData;
	AChunkWorld* chunkWorld;
	APerlinNoiseSettings* perlinNoiseSettings;


protected:
	// Called when the game starts or when spawned
	virtual void Init() override;

};
