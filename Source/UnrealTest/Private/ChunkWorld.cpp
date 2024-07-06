// Fill out your copyright notice in the Description page of Project Settings.


#include "ChunkWorld.h"

// Sets default values
AChunkWorld::AChunkWorld()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

void AChunkWorld::printExecutionTime(Time& start, Time& end, const char* functionName) {
	std::chrono::duration<double, std::milli> duration = end - start;
	UE_LOG(LogTemp, Warning, TEXT("%s() took %f milliseconds to execute."), *FString(functionName), duration.count());
}

// Called when the game starts or when spawned
void AChunkWorld::BeginPlay()
{
	Super::BeginPlay();


	// FVector PlayerPosition = GetWorld()->GetFirstPlayerController()->GetPawn()->GetActorLocation();


	int spawnedChunks{ 0 };
	Time start = std::chrono::high_resolution_clock::now();

	for (int x = -DrawDistance; x < DrawDistance; x++) {
		for (int y = -DrawDistance; y < DrawDistance; y++) {
			GetWorld()->SpawnActor<AActor>(Chunk, FVector(x * ChunkSize * 100, y * ChunkSize * 100, 0), FRotator::ZeroRotator);
			spawnedChunks++;
		}
	}

	Time end = std::chrono::high_resolution_clock::now();

	printExecutionTime(start, end, "Spawned entire terrain.");
	UE_LOG(LogTemp, Warning, TEXT("Chunks spawned: %d"), spawnedChunks);

	
}

