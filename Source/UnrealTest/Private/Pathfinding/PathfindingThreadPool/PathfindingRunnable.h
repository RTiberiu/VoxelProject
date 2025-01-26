// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Async/Async.h"
#include "CoreMinimal.h"
#include <HAL/Platform.h>

class UWorldTerrainSettings; // forward declaration to the world settings
class UChunkLocationData;

class PathfindingRunnable : public FRunnable {
public:
	PathfindingRunnable(UWorldTerrainSettings* InWorldTerrainSettingsRef, UChunkLocationData* InChunkLocationDataRef);
	virtual ~PathfindingRunnable() override;

	virtual bool Init() override;
	virtual uint32 Run() override;
	virtual void Stop() override;
	virtual void Exit() override;

	void SetSearchData(FVector* InStartLocation, FVector* InEndLocation);

	FThreadSafeBool IsRunning() const;

	void SetWorldTerrainSettings(UWorldTerrainSettings* InWorldTerrainSettings);
	void SetChunkLocationData(UChunkLocationData* InChunkLocationData);

private:
	UWorldTerrainSettings* WorldTerrainSettingsRef;
	UWorldTerrainSettings*& WTSR = WorldTerrainSettingsRef;
	UChunkLocationData* ChunkLocationDataRef;
	UChunkLocationData*& CLDR = ChunkLocationDataRef;

	FVector* startLocation;
	FVector* endLocation;

	FThreadSafeBool isRunning;
};
