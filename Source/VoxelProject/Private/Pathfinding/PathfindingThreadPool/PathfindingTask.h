// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Async/Async.h"
#include "CoreMinimal.h"
#include "..\..\NPC\BasicNPC\BasicNPC.h"
#include "PathfindingThreadManager.h"
#include "..\SearchProblem\VoxelSearchProblem.h"

class ABasicNPC;
class VoxelSearchProblem;
class UWorldTerrainSettings;
class UChunkLocationData;

class FPathfindingTask : public IQueuedWork {
public:
	FPathfindingTask(
		const FVector& InStartLocation, 
		const FVector& InEndLocation,
		ABasicNPC* InNPCRef,
		UWorldTerrainSettings* InWorldTerrainSettingsRef, 
		UChunkLocationData* InChunkLocationDataRef
		);

	virtual ~FPathfindingTask();

	virtual void DoThreadedWork() override;
	virtual void Abandon() override;


	void SetWorldTerrainSettings(UWorldTerrainSettings* InWorldTerrainSettings);
	void SetChunkLocationData(UChunkLocationData* InChunkLocationData);

private:
	void AdjustLocationsToUnrealScaling();
	Path* GetPathToEndLocation();
	void AdjustPathWithActualVoxelHeights(Path* path);

	void PrintHeights(const TArray<int>& heights); // TESTING THE SURFACE VOXELS

	UWorldTerrainSettings* WorldTerrainSettingsRef;
	UWorldTerrainSettings*& WTSR = WorldTerrainSettingsRef;

	UChunkLocationData* ChunkLocationDataRef;
	UChunkLocationData*& CLDR = ChunkLocationDataRef;

	ABasicNPC* NPCRef;

	FVector StartLocation;
	FVector EndLocation;

	VoxelSearchProblem* searchProblem;
	bool isSearching;
};
