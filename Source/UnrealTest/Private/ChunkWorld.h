// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <chrono>
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ChunkWorld.generated.h"

UCLASS()
class AChunkWorld : public AActor {
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AChunkWorld();

	void UpdateChunks();

	UPROPERTY(EditAnywhere, Category = "Chunk World")
	TSubclassOf<AActor> Chunk;

	UPROPERTY(EditAnywhere, Category = "Chunk World")
	int DrawDistance = 2;

	UPROPERTY(EditAnywhere, Category = "Chunk World")
	int ChunkSize = 62;

	int UnrealScale = 100;



private:
	// Create chrono type alias // TODO This is a duplicate from BinaryChunk.h; Add this in a UTIL header
	using Time = std::chrono::high_resolution_clock::time_point;


	void printExecutionTime(Time& start, Time& end, const char* functionName); // TODO This is a duplicate from BinaryChunk.h; Add this in a UTIL header

	void spawnInitialWorld();

	FIntPoint GetChunkCoordinates(FVector Position) const;

	FVector playerInitialPosition;

	// Map to store spawned chunks with 2D coordinates as keys
	TMap<FIntPoint, AActor*> SpawnedChunksMap;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaSeconds) override;
};
