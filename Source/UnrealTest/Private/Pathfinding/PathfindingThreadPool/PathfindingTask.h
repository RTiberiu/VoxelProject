// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Async/Async.h"
#include "CoreMinimal.h"
#include "PathfindingThreadManager.h"

class UWorldTerrainSettings;
class UChunkLocationData;

class FPathfindingTask : public IQueuedWork {
public:
	/*PathfindingRunnable(UWorldTerrainSettings* InWorldTerrainSettingsRef, UChunkLocationData* InChunkLocationDataRef);
	virtual ~PathfindingRunnable() override;*/

	FPathfindingTask(
		const FVector& InStartLocation, 
		const FVector& InEndLocation,
		UWorldTerrainSettings* InWorldTerrainSettingsRef, 
		UChunkLocationData* InChunkLocationDataRef
		);

	virtual ~FPathfindingTask() {}

	virtual void DoThreadedWork() override;
	virtual void Abandon() override;


	void SetWorldTerrainSettings(UWorldTerrainSettings* InWorldTerrainSettings);
	void SetChunkLocationData(UChunkLocationData* InChunkLocationData);

private:
	void TestFakeSearch();

	UWorldTerrainSettings* WorldTerrainSettingsRef;
	UWorldTerrainSettings*& WTSR = WorldTerrainSettingsRef;

	UChunkLocationData* ChunkLocationDataRef;
	UChunkLocationData*& CLDR = ChunkLocationDataRef;

	FVector StartLocation;
	FVector EndLocation;

};
