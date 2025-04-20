// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Async/Async.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "..\..\Utils\CustomMesh\CustomProceduralMeshComponent.h"

class ATree;
class ABinaryChunk;
class UWorldTerrainSettings; // forward declaration to the world settings
class UChunkLocationData;

enum SpawnPointType {
	VEGETATION,
	CHUNKS,
	TREES
};

class ChunksLocationRunnable : public FRunnable {
public:
	ChunksLocationRunnable(
		FVector PlayerPosition, 
		UWorldTerrainSettings* InWorldTerrainSettingsRef, 
		UChunkLocationData* InChunkLocationDataRef,
		TQueue<UCustomProceduralMeshComponent*>* InGrassActorsToRemove,
		TQueue<UCustomProceduralMeshComponent*>* InFlowerActorsToRemove,
		TQueue<ATree*>* InTreeActorsToRemove
		);
	virtual ~ChunksLocationRunnable() override;

	virtual bool Init() override;
	virtual uint32 Run() override;
	virtual void Stop() override;
	virtual void Exit() override;

	FThreadSafeBool IsTaskComplete() const;

	void SetWorldTerrainSettings(UWorldTerrainSettings* InWorldTerrainSettings);
	void SetChunkLocationData(UChunkLocationData* InChunkLocationData);

private:
	UWorldTerrainSettings* WorldTerrainSettingsRef;
	UWorldTerrainSettings*& WTSR = WorldTerrainSettingsRef;
	UChunkLocationData* ChunkLocationDataRef;
	UChunkLocationData*& CLDR = ChunkLocationDataRef;

	TQueue<UCustomProceduralMeshComponent*>* GrassActorsToRemove;
	TQueue<UCustomProceduralMeshComponent*>* FlowerActorsToRemove;
	TQueue<ATree*>* TreeActorsToRemove;

	FVector PlayerPosition;

	void UpdateSpawnPoints(SpawnPointType SpawnType);

	void UpdateChunkCollisions();
	void UpdateTreeCollisions();

	FIntPoint GetChunkCoordinates(FVector Position) const;

	FThreadSafeBool isRunning;
	FThreadSafeBool isTaskComplete;
};
