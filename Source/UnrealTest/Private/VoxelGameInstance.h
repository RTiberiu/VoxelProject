// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Noise\PerlinNoiseSettings.h"
#include "Chunks\TerrainSettings\WorldTerrainSettings.h"
#include "Chunks\ChunkData\ChunkLocationData.h"
#include "Chunks\ChunkWorld\ChunkWorld.h"
#include "NPC/SettingsNPC/AnimationSettingsNPC.h"
#include "NPC/StatsNPC/StatsMeshGenerator.h"
#include "NPC/StatsNPC/StatsVoxelsMeshNPC.h"

#include "CoreMinimal.h"
//#include "Engine/GameInstance.h"
#include "VoxelGameInstance.generated.h"

UCLASS()
class AVoxelGameInstance : public AActor {
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AVoxelGameInstance();

private:
	UWorldTerrainSettings* worldTerrainSettings;
	UChunkLocationData* chunkLocationData;
	AChunkWorld* chunkWorld;
	APerlinNoiseSettings* perlinNoiseSettings;
	UAnimationSettingsNPC* animationSettingsNpcRef;
	UStatsVoxelsMeshNPC* statsVoxelsMeshNPC;

	void GenerateStatsVoxelMeshes();
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
};
