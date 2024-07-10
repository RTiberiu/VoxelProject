// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <chrono>
#include "TerrainRunnable.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ChunkWorld.generated.h"

UCLASS()
class AChunkWorld : public AActor {
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AChunkWorld();

	UPROPERTY(EditAnywhere, Category = "Chunk World")
	TSubclassOf<AActor> Chunk;

private:
	// Create chrono type alias // TODO This is a duplicate from BinaryChunk.h; Add this in a UTIL header
	using Time = std::chrono::high_resolution_clock::time_point;


	void printExecutionTime(Time& start, Time& end, const char* functionName); // TODO This is a duplicate from BinaryChunk.h; Add this in a UTIL header

	// Ensure that the Tick() function runs after the BeginPlay() is done initializing the world
	FThreadSafeBool isInitialWorldGenerated;

	void spawnInitialWorld();

	// Runnable to handle spawning the chunks
	TerrainRunnable* terrainRunnable;
	FRunnableThread* terrainRunnableThread;
	FThreadSafeBool isTaskRunning;
	
	// Handle 
	void onNewTerrainGenerated();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	FIntPoint GetChunkCoordinates(FVector Position) const;

	virtual void Tick(float DeltaSeconds) override;
};
