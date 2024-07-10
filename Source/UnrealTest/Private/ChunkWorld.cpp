// Fill out your copyright notice in the Description page of Project Settings.


#include "ChunkWorld.h"
#include "GameFramework/DefaultPawn.h"
#include "GameFramework/FloatingPawnMovement.h"

// Sets default values
AChunkWorld::AChunkWorld() : terrainRunnable(nullptr), terrainRunnableThread(nullptr), isTaskRunning(false) {
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	isInitialWorldGenerated = false;

	// TerrainRunnable::setDrawDistance(5);
}

void AChunkWorld::printExecutionTime(Time& start, Time& end, const char* functionName) {
	std::chrono::duration<double, std::milli> duration = end - start;
	UE_LOG(LogTemp, Warning, TEXT("%s() took %f milliseconds to execute."), *FString(functionName), duration.count());
}

void AChunkWorld::spawnInitialWorld() {
	TerrainRunnable::EmptyChunkMap(); // Clear the map of chunks TODO MAYBE DELETE THIS LINE I HAVE A PROBLEM WITH 32 CHUNKS INSTEAD OF 16

	int spawnedChunks{ 0 };
	Time start = std::chrono::high_resolution_clock::now();

	for (int x = -DrawDistance.load(); x < DrawDistance.load(); x++) {
		for (int z = -DrawDistance.load(); z < DrawDistance.load(); z++) {
			FVector ChunkPosition = FVector(x * chunkSize * UnrealScale, z * chunkSize * UnrealScale, 0);
            AActor* SpawnedChunk = GetWorld()->SpawnActor<AActor>(Chunk, ChunkPosition, FRotator::ZeroRotator);
            if (SpawnedChunk) {
				UE_LOG(LogTemp, Warning, TEXT("Chunk counter: %d"), spawnedChunks);
				TerrainRunnable::AddChunkToMap(FIntPoint(x, z), SpawnedChunk);
				spawnedChunks++;
            }
		}
	}

	Time end = std::chrono::high_resolution_clock::now();

	printExecutionTime(start, end, "Spawned entire terrain.");
	UE_LOG(LogTemp, Warning, TEXT("Chunks spawned: %d"), spawnedChunks);
	UE_LOG(LogTemp, Warning, TEXT("SpawnedChunkMap size = %d"), TerrainRunnable::GetMapSize());
	UE_LOG(LogTemp, Warning, TEXT("DrawDistance = %d"), DrawDistance.load());

	// Print SpawnedChunksMap keys before destroying
	TerrainRunnable::printMapElements("SpawnedChunksMap Keys AFTER spawnInitialWorld");
}



// Perform any actions after generating the new chunks
void AChunkWorld::onNewTerrainGenerated() {
}

// Called when the game starts or when spawned
void AChunkWorld::BeginPlay() {
	Super::BeginPlay();

	spawnInitialWorld();
	
	// Set player's initial position
	TerrainRunnable::updateInitialPlayerPosition(GetWorld()->GetFirstPlayerController()->GetPawn()->GetActorLocation());

	// Testing position
	FIntPoint playerPositionTest = GetChunkCoordinates(TerrainRunnable::getInitialPlayerPosition());
	UE_LOG(LogTemp, Warning, TEXT("Initialized playerInitialPosition to X=%d Z=%d"), static_cast<int>(playerPositionTest.X), static_cast<int>(playerPositionTest.Y));

	// Set the player's maximum speed 
	ADefaultPawn* PlayerPawn = Cast<ADefaultPawn>(GetWorld()->GetFirstPlayerController()->GetPawn());
	if (PlayerPawn) {
		UFloatingPawnMovement* MovementComponent = Cast<UFloatingPawnMovement>(PlayerPawn->GetMovementComponent());
		if (MovementComponent) {
			MovementComponent->MaxSpeed = 10000.0f;
			UE_LOG(LogTemp, Warning, TEXT("Player speed set to %f"), MovementComponent->MaxSpeed);
		} else {
			UE_LOG(LogTemp, Error, TEXT("Player pawn does not have a movement component!"));
		}
	} else {
		UE_LOG(LogTemp, Error, TEXT("Player pawn is not a DefaultPawn!"));
	}

	isInitialWorldGenerated = true;
}

FIntPoint AChunkWorld::GetChunkCoordinates(FVector Position) const {
	int32 ChunkX = FMath::FloorToInt(Position.X / (chunkSize * UnrealScale));
	int32 ChunkY = FMath::FloorToInt(Position.Y / (chunkSize * UnrealScale));
	return FIntPoint(ChunkX, ChunkY);
}

// Called every frame
void AChunkWorld::Tick(float DeltaSeconds) {
	Super::Tick(DeltaSeconds);

	// Continue running only if the BeginPlay() is done initializing the world
	if (!isInitialWorldGenerated) {
		UE_LOG(LogTemp, Warning, TEXT("World not yet initialized. Tick() will exit now."));
		return;
	}

	TickCriticalSection.Lock();

	FVector PlayerPosition = GetWorld()->GetFirstPlayerController()->GetPawn()->GetActorLocation();

	FIntPoint PlayerChunkCoords = GetChunkCoordinates(PlayerPosition);
	FIntPoint InitialChunkCoords = GetChunkCoordinates(TerrainRunnable::getInitialPlayerPosition());

	bool isPlayerMovingOnAxisX = PlayerChunkCoords.X != InitialChunkCoords.X;
	bool isPlayerMovingOnAxisZ = PlayerChunkCoords.Y != InitialChunkCoords.Y;


	if (!isTaskRunning && (isPlayerMovingOnAxisX || isPlayerMovingOnAxisZ)) {
		isTaskRunning = true;

		UE_LOG(LogTemp, Warning, TEXT("Created new thread!"));

		terrainRunnable = new TerrainRunnable(GetWorld(), &Chunk);
		terrainRunnableThread = FRunnableThread::Create(terrainRunnable, TEXT("terrainRunnableThread"), 0, TPri_Normal);
	}

	if (terrainRunnable && terrainRunnable->IsTaskComplete()) {
		UE_LOG(LogTemp, Warning, TEXT("Destroyed new thread!"));

		onNewTerrainGenerated();

		// Clean up
		if (terrainRunnableThread) {
			terrainRunnableThread->Kill(true);
			delete terrainRunnableThread;
			terrainRunnableThread = nullptr;
		}
		if (terrainRunnable) {
			delete terrainRunnable;
			terrainRunnable = nullptr;
		}

		isTaskRunning = false;
	}

	TickCriticalSection.Unlock();

}

