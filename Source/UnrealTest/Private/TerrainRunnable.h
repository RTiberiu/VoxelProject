// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "WorldTerrainSettings.h"
#include "Async/Async.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

/**
 *
 */
class TerrainRunnable : public FRunnable {
public:
	TerrainRunnable(UWorld* World, TSubclassOf<AActor>* Chunk);
	virtual ~TerrainRunnable() override;

	virtual bool Init() override;
	virtual uint32 Run() override;
	virtual void Stop() override;
	virtual void Exit() override;

	FThreadSafeBool IsTaskComplete() const;

	// Methods to interact with the world chunks
	static void AddChunkToMap(const FIntPoint& ChunkCoordinates, AActor* ChunkActor);
	static AActor* GetAndRemoveChunkFromMap(const FIntPoint& ChunkCoordinates);
	static int GetMapSize();
	static void updateInitialPlayerPosition(FVector newPosition);
	static FVector getInitialPlayerPosition();
	static void EmptyChunkMap();
	static void printMapElements(FString message);

private:
	UWorld* World;
	TSubclassOf<AActor>* Chunk;

	FVector PlayerPosition;

	void UpdateChunks();

	FIntPoint GetChunkCoordinates(FVector Position) const;

	FThreadSafeBool isRunning;
	FThreadSafeBool isTaskComplete;
};
